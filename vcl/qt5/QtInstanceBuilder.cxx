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

#include <QtAccessibleWidget.hxx>
#include <QtBuilder.hxx>
#include <QtInstanceAssistant.hxx>
#include <QtInstanceBox.hxx>
#include <QtInstanceCheckButton.hxx>
#include <QtInstanceComboBox.hxx>
#include <QtInstanceDrawingArea.hxx>
#include <QtInstanceEntry.hxx>
#include <QtInstanceEntryTreeView.hxx>
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
#include <QtInstanceScrollbar.hxx>
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
        u"cui/ui/breaknumberoption.ui"_ustr,
        u"cui/ui/fontfeaturesdialog.ui"_ustr,
        u"cui/ui/fontfragment.ui"_ustr,
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
        u"cui/ui/selectpathdialog.ui"_ustr,
        u"cui/ui/spelloptionsdialog.ui"_ustr,
        u"cui/ui/splitcellsdialog.ui"_ustr,
        u"cui/ui/tipofthedaydialog.ui"_ustr,
        u"cui/ui/toolbarmodedialog.ui"_ustr,
        u"cui/ui/zoomdialog.ui"_ustr,
        u"dbaccess/ui/savedialog.ui"_ustr,
        u"dbaccess/ui/tabledesignsavemodifieddialog.ui"_ustr,
        u"filter/ui/testxmlfilter.ui"_ustr,
        u"filter/ui/xmlfiltertabpagegeneral.ui"_ustr,
        u"filter/ui/xmlfiltertabpagetransformation.ui"_ustr,
        u"filter/ui/xmlfiltersettings.ui"_ustr,
        u"filter/ui/xsltfilterdialog.ui"_ustr,
        u"modules/BasicIDE/ui/gotolinedialog.ui"_ustr,
        u"modules/scalc/ui/inputstringdialog.ui"_ustr,
        u"modules/scalc/ui/insertcells.ui"_ustr,
        u"modules/scalc/ui/movecopysheet.ui"_ustr,
        u"modules/scalc/ui/optdlg.ui"_ustr,
        u"modules/scalc/ui/selectsource.ui"_ustr,
        u"modules/scalc/ui/showsheetdialog.ui"_ustr,
        u"modules/schart/ui/insertaxisdlg.ui"_ustr,
        u"modules/simpress/ui/presentationdialog.ui"_ustr,
        u"modules/smath/ui/alignmentdialog.ui"_ustr,
        u"modules/smath/ui/fontdialog.ui"_ustr,
        u"modules/smath/ui/fontsizedialog.ui"_ustr,
        u"modules/smath/ui/fonttypedialog.ui"_ustr,
        u"modules/smath/ui/savedefaultsdialog.ui"_ustr,
        u"modules/smath/ui/smathsettings.ui"_ustr,
        u"modules/smath/ui/spacingdialog.ui"_ustr,
        u"modules/spropctrlr/ui/taborder.ui"_ustr,
        u"modules/swriter/ui/authenticationsettingsdialog.ui"_ustr,
        u"modules/swriter/ui/columnwidth.ui"_ustr,
        u"modules/swriter/ui/editcategories.ui"_ustr,
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
        u"svt/ui/addresstemplatedialog.ui"_ustr,
        u"svt/ui/printersetupdialog.ui"_ustr,
        u"svt/ui/restartdialog.ui"_ustr,
        u"svx/ui/compressgraphicdialog.ui"_ustr,
        u"svx/ui/docrecoverysavedialog.ui"_ustr,
        u"svx/ui/fontworkgallerydialog.ui"_ustr,
        u"svx/ui/deletefooterdialog.ui"_ustr,
        u"svx/ui/deleteheaderdialog.ui"_ustr,
        u"svx/ui/gotopagedialog.ui"_ustr,
        u"svx/ui/safemodedialog.ui"_ustr,
        u"uui/ui/password.ui"_ustr,
        u"vcl/ui/openlockedquerybox.ui"_ustr,
        u"vcl/ui/printerdevicepage.ui"_ustr,
        u"vcl/ui/printerpaperpage.ui"_ustr,
        u"vcl/ui/printerpropertiesdialog.ui"_ustr,
        u"vcl/ui/printprogressdialog.ui"_ustr,
        u"xmlsec/ui/macrosecuritydialog.ui"_ustr,
        u"xmlsec/ui/securitylevelpage.ui"_ustr,
        u"xmlsec/ui/securitytrustpage.ui"_ustr,
        u"writerperfect/ui/exportepub.ui"_ustr,
    };

    return aSupportedUIFiles.contains(rUIFile);
}

std::unique_ptr<weld::MessageDialog> QtInstanceBuilder::weld_message_dialog(const OUString& id)
{
    QMessageBox* pMessageBox = m_xBuilder->get<QMessageBox>(id);
    assert(pMessageBox);
    return std::make_unique<QtInstanceMessageDialog>(pMessageBox);
}

std::unique_ptr<weld::Dialog> QtInstanceBuilder::weld_dialog(const OUString& rId)
{
    QDialog* pDialog = m_xBuilder->get<QDialog>(rId);
    assert(pDialog);
    return std::make_unique<QtInstanceDialog>(pDialog);
}

std::unique_ptr<weld::Assistant> QtInstanceBuilder::weld_assistant(const OUString& rId)
{
    QWizard* pWizard = m_xBuilder->get<QWizard>(rId);
    assert(pWizard);
    return std::make_unique<QtInstanceAssistant>(pWizard);
}

std::unique_ptr<weld::Window> QtInstanceBuilder::create_screenshot_window()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Widget> QtInstanceBuilder::weld_widget(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    assert(pWidget);
    return std::make_unique<QtInstanceWidget>(pWidget);
}

std::unique_ptr<weld::Container> QtInstanceBuilder::weld_container(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    assert(pWidget);
    assert(pWidget->layout() && "no layout");
    return std::make_unique<QtInstanceContainer>(pWidget);
}

std::unique_ptr<weld::Box> QtInstanceBuilder::weld_box(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    assert(pWidget);
    assert(qobject_cast<QBoxLayout*>(pWidget->layout()) && "widget doesn't have a box layout");
    return std::make_unique<QtInstanceBox>(pWidget);
}

std::unique_ptr<weld::Grid> QtInstanceBuilder::weld_grid(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    assert(pWidget);
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
    assert(pGroupBox);
    return std::make_unique<QtInstanceFrame>(pGroupBox);
}

std::unique_ptr<weld::ScrolledWindow> QtInstanceBuilder::weld_scrolled_window(const OUString& rId,
                                                                              bool)
{
    QScrollArea* pScrollArea = m_xBuilder->get<QScrollArea>(rId);
    assert(pScrollArea);
    return std::make_unique<QtInstanceScrolledWindow>(pScrollArea);
}

std::unique_ptr<weld::Notebook> QtInstanceBuilder::weld_notebook(const OUString& rId)
{
    QTabWidget* pTabWidget = m_xBuilder->get<QTabWidget>(rId);
    assert(pTabWidget);
    return std::make_unique<QtInstanceNotebook>(pTabWidget);
}

std::unique_ptr<weld::Button> QtInstanceBuilder::weld_button(const OUString& rId)
{
    QPushButton* pButton = m_xBuilder->get<QPushButton>(rId);
    assert(pButton);
    return std::make_unique<QtInstanceButton>(pButton);
}

std::unique_ptr<weld::MenuButton> QtInstanceBuilder::weld_menu_button(const OUString& rId)
{
    QToolButton* pButton = m_xBuilder->get<QToolButton>(rId);
    assert(pButton);
    return std::make_unique<QtInstanceMenuButton>(pButton);
}

std::unique_ptr<weld::MenuToggleButton> QtInstanceBuilder::weld_menu_toggle_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::LinkButton> QtInstanceBuilder::weld_link_button(const OUString& rId)
{
    QtHyperlinkLabel* pLabel = m_xBuilder->get<QtHyperlinkLabel>(rId);
    assert(pLabel);
    return std::make_unique<QtInstanceLinkButton>(pLabel);
}

std::unique_ptr<weld::ToggleButton> QtInstanceBuilder::weld_toggle_button(const OUString& rId)
{
    QAbstractButton* pButton = m_xBuilder->get<QAbstractButton>(rId);
    assert(pButton);
    return std::make_unique<QtInstanceToggleButton>(pButton);
}

std::unique_ptr<weld::RadioButton> QtInstanceBuilder::weld_radio_button(const OUString& rId)
{
    QRadioButton* pRadioButton = m_xBuilder->get<QRadioButton>(rId);
    assert(pRadioButton);
    return std::make_unique<QtInstanceRadioButton>(pRadioButton);
}

std::unique_ptr<weld::CheckButton> QtInstanceBuilder::weld_check_button(const OUString& rId)
{
    QCheckBox* pCheckBox = m_xBuilder->get<QCheckBox>(rId);
    assert(pCheckBox);
    return std::make_unique<QtInstanceCheckButton>(pCheckBox);
}

std::unique_ptr<weld::Scale> QtInstanceBuilder::weld_scale(const OUString& rId)
{
    QSlider* pSlider = m_xBuilder->get<QSlider>(rId);
    assert(pSlider);
    return std::make_unique<QtInstanceScale>(pSlider);
}

std::unique_ptr<weld::ProgressBar> QtInstanceBuilder::weld_progress_bar(const OUString& rId)
{
    QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId);
    assert(pProgressBar);
    return std::make_unique<QtInstanceProgressBar>(pProgressBar);
}

std::unique_ptr<weld::LevelBar> QtInstanceBuilder::weld_level_bar(const OUString& rId)
{
    QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId);
    assert(pProgressBar);
    return std::make_unique<QtInstanceLevelBar>(pProgressBar);
}

std::unique_ptr<weld::Spinner> QtInstanceBuilder::weld_spinner(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Image> QtInstanceBuilder::weld_image(const OUString& rId)
{
    QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
    assert(pLabel);
    return std::make_unique<QtInstanceImage>(pLabel);
}

std::unique_ptr<weld::Calendar> QtInstanceBuilder::weld_calendar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Entry> QtInstanceBuilder::weld_entry(const OUString& rId)
{
    QLineEdit* pLineEdit = m_xBuilder->get<QLineEdit>(rId);
    assert(pLineEdit);
    return std::make_unique<QtInstanceEntry>(pLineEdit);
}

std::unique_ptr<weld::SpinButton> QtInstanceBuilder::weld_spin_button(const OUString& rId)
{
    QtDoubleSpinBox* pSpinBox = m_xBuilder->get<QtDoubleSpinBox>(rId);
    assert(pSpinBox);
    return std::make_unique<QtInstanceSpinButton>(pSpinBox);
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
    assert(pSpinBox);
    return std::make_unique<QtInstanceFormattedSpinButton>(pSpinBox);
}

std::unique_ptr<weld::ComboBox> QtInstanceBuilder::weld_combo_box(const OUString& rId)
{
    QComboBox* pComboBox = m_xBuilder->get<QComboBox>(rId);
    assert(pComboBox);
    return std::make_unique<QtInstanceComboBox>(pComboBox);
}

std::unique_ptr<weld::EntryTreeView>
QtInstanceBuilder::weld_entry_tree_view(const OUString& rContainerId, const OUString& rEntryId,
                                        const OUString& rTreeViewId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rContainerId);
    QLineEdit* pLineEdit = m_xBuilder->get<QLineEdit>(rEntryId);
    QTreeView* pTreeView = m_xBuilder->get<QTreeView>(rTreeViewId);
    assert(pWidget && pLineEdit && pTreeView);

    return std::make_unique<QtInstanceEntryTreeView>(
        pWidget, pLineEdit, pTreeView, weld_entry(rEntryId), weld_tree_view(rTreeViewId));
}

std::unique_ptr<weld::TreeView> QtInstanceBuilder::weld_tree_view(const OUString& rId)
{
    QTreeView* pTreeView = m_xBuilder->get<QTreeView>(rId);
    assert(pTreeView);
    return std::make_unique<QtInstanceTreeView>(pTreeView);
}

std::unique_ptr<weld::IconView> QtInstanceBuilder::weld_icon_view(const OUString& rId)
{
    QListView* pListView = m_xBuilder->get<QListView>(rId);
    assert(pListView);
    return std::make_unique<QtInstanceIconView>(pListView);
}

std::unique_ptr<weld::Label> QtInstanceBuilder::weld_label(const OUString& rId)
{
    QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
    assert(pLabel);
    return std::make_unique<QtInstanceLabel>(pLabel);
}

std::unique_ptr<weld::TextView> QtInstanceBuilder::weld_text_view(const OUString& rId)
{
    QPlainTextEdit* pTextEdit = m_xBuilder->get<QPlainTextEdit>(rId);
    assert(pTextEdit);
    return std::make_unique<QtInstanceTextView>(pTextEdit);
}

std::unique_ptr<weld::Expander> QtInstanceBuilder::weld_expander(const OUString& rId)
{
    QtExpander* pExpander = m_xBuilder->get<QtExpander>(rId);
    assert(pExpander);
    return std::make_unique<QtInstanceExpander>(pExpander);
}

std::unique_ptr<weld::DrawingArea> QtInstanceBuilder::weld_drawing_area(const OUString& rId,
                                                                        const a11yref& rA11yImpl,
                                                                        FactoryFunction, void*)
{
    QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
    assert(pLabel);

    if (rA11yImpl.is())
        QtAccessibleWidget::setCustomAccessible(*pLabel, rA11yImpl);

    return std::make_unique<QtInstanceDrawingArea>(pLabel);
}

std::unique_ptr<weld::Menu> QtInstanceBuilder::weld_menu(const OUString& rId)
{
    QMenu* pMenu = m_xBuilder->get_menu(rId);
    assert(pMenu);
    return std::make_unique<QtInstanceMenu>(pMenu);
}

std::unique_ptr<weld::Popover> QtInstanceBuilder::weld_popover(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    assert(pWidget);
    return std::make_unique<QtInstancePopover>(pWidget);
}

std::unique_ptr<weld::Toolbar> QtInstanceBuilder::weld_toolbar(const OUString& rId)
{
    QToolBar* pToolBar = m_xBuilder->get<QToolBar>(rId);
    assert(pToolBar);
    return std::make_unique<QtInstanceToolbar>(pToolBar);
}

std::unique_ptr<weld::Scrollbar> QtInstanceBuilder::weld_scrollbar(const OUString& rId)
{
    QScrollBar* pScrollBar = m_xBuilder->get<QScrollBar>(rId);
    assert(pScrollBar);
    return std::make_unique<QtInstanceScrollbar>(pScrollBar);
}

std::unique_ptr<weld::SizeGroup> QtInstanceBuilder::create_size_group()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
