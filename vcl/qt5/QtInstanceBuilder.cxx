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
#include <QtInstanceAssistant.hxx>
#include <QtInstanceBox.hxx>
#include <QtInstanceCalendar.hxx>
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
#include <QtInstanceSpinner.hxx>
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

bool QtInstanceBuilder::IsUIFileSupported(const OUString& rUIFile, const weld::Widget* pParent)
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
        u"cui/ui/editdictionarydialog.ui"_ustr,
        u"cui/ui/fontfeaturesdialog.ui"_ustr,
        u"cui/ui/fontfragment.ui"_ustr,
        u"cui/ui/graphictestdlg.ui"_ustr,
        u"cui/ui/imageviewer.ui"_ustr,
        u"cui/ui/insertrowcolumn.ui"_ustr,
        u"cui/ui/javastartparametersdialog.ui"_ustr,
        u"cui/ui/linedialog.ui"_ustr,
        u"cui/ui/namedialog.ui"_ustr,
        u"cui/ui/numberdialog.ui"_ustr,
        u"cui/ui/objecttitledescdialog.ui"_ustr,
        u"cui/ui/objectnamedialog.ui"_ustr,
        u"cui/ui/optlingupage.ui"_ustr,
        u"cui/ui/optnewdictionarydialog.ui"_ustr,
        u"cui/ui/password.ui"_ustr,
        u"cui/ui/pastespecial.ui"_ustr,
        u"cui/ui/positionsizedialog.ui"_ustr,
        u"cui/ui/qrcodegen.ui"_ustr,
        u"cui/ui/querydeletedictionarydialog.ui"_ustr,
        u"cui/ui/querydeletelineenddialog.ui"_ustr,
        u"cui/ui/querydeletelinestyledialog.ui"_ustr,
        u"cui/ui/querysavelistdialog.ui"_ustr,
        u"cui/ui/querydialog.ui"_ustr,
        u"cui/ui/queryduplicatedialog.ui"_ustr,
        u"cui/ui/searchattrdialog.ui"_ustr,
        u"cui/ui/securityoptionsdialog.ui"_ustr,
        u"cui/ui/selectpathdialog.ui"_ustr,
        u"cui/ui/signatureline.ui"_ustr,
        u"cui/ui/signsignatureline.ui"_ustr,
        u"cui/ui/similaritysearchdialog.ui"_ustr,
        u"cui/ui/spelloptionsdialog.ui"_ustr,
        u"cui/ui/splitcellsdialog.ui"_ustr,
        u"cui/ui/textdialog.ui"_ustr,
        u"cui/ui/tipofthedaydialog.ui"_ustr,
        u"cui/ui/toolbartabpage.ui"_ustr,
        u"cui/ui/uipickerdialog.ui"_ustr,
        u"cui/ui/uitabpage.ui"_ustr,
        u"cui/ui/welcomedialog.ui"_ustr,
        u"cui/ui/whatsnewtabpage.ui"_ustr,
        u"cui/ui/widgettestdialog.ui"_ustr,
        u"cui/ui/zoomdialog.ui"_ustr,
        u"dbaccess/ui/savedialog.ui"_ustr,
        u"dbaccess/ui/tabledesignsavemodifieddialog.ui"_ustr,
        u"desktop/ui/installforalldialog.ui"_ustr,
        u"desktop/ui/licensedialog.ui"_ustr,
        u"filter/ui/testxmlfilter.ui"_ustr,
        u"filter/ui/xmlfiltertabpagegeneral.ui"_ustr,
        u"filter/ui/xmlfiltertabpagetransformation.ui"_ustr,
        u"filter/ui/xmlfiltersettings.ui"_ustr,
        u"filter/ui/xsltfilterdialog.ui"_ustr,
        u"modules/BasicIDE/ui/gotolinedialog.ui"_ustr,
        u"modules/BasicIDE/ui/newlibdialog.ui"_ustr,
        u"modules/scalc/ui/gotosheetdialog.ui"_ustr,
        u"modules/scalc/ui/groupbydate.ui"_ustr,
        u"modules/scalc/ui/inputstringdialog.ui"_ustr,
        u"modules/scalc/ui/insertcells.ui"_ustr,
        u"modules/scalc/ui/movecopysheet.ui"_ustr,
        u"modules/scalc/ui/optdlg.ui"_ustr,
        u"modules/scalc/ui/selectsource.ui"_ustr,
        u"modules/scalc/ui/showsheetdialog.ui"_ustr,
        u"modules/schart/ui/insertaxisdlg.ui"_ustr,
        u"modules/sdraw/ui/dlgsnap.ui"_ustr,
        u"modules/sdraw/ui/insertlayer.ui"_ustr,
        u"modules/simpress/ui/customslideshows.ui"_ustr,
        u"modules/simpress/ui/definecustomslideshow.ui"_ustr,
        u"modules/simpress/ui/masterlayoutdlg.ui"_ustr,
        u"modules/simpress/ui/presentationdialog.ui"_ustr,
        u"modules/simpress/ui/slidedesigndialog.ui"_ustr,
        u"modules/smath/ui/alignmentdialog.ui"_ustr,
        u"modules/smath/ui/fontdialog.ui"_ustr,
        u"modules/smath/ui/fontsizedialog.ui"_ustr,
        u"modules/smath/ui/fonttypedialog.ui"_ustr,
        u"modules/smath/ui/savedefaultsdialog.ui"_ustr,
        u"modules/smath/ui/smathsettings.ui"_ustr,
        u"modules/smath/ui/spacingdialog.ui"_ustr,
        u"modules/spropctrlr/ui/taborder.ui"_ustr,
        u"modules/swriter/ui/authenticationsettingsdialog.ui"_ustr,
        u"modules/swriter/ui/captionoptions.ui"_ustr,
        u"modules/swriter/ui/columnwidth.ui"_ustr,
        u"modules/swriter/ui/editcategories.ui"_ustr,
        u"modules/swriter/ui/endnotepage.ui"_ustr,
        u"modules/swriter/ui/footendnotedialog.ui"_ustr,
        u"modules/swriter/ui/footnotepage.ui"_ustr,
        u"modules/swriter/ui/inforeadonlydialog.ui"_ustr,
        u"modules/swriter/ui/insertbreak.ui"_ustr,
        u"modules/swriter/ui/inserttable.ui"_ustr,
        u"modules/swriter/ui/linenumbering.ui"_ustr,
        u"modules/swriter/ui/numberingnamedialog.ui"_ustr,
        u"modules/swriter/ui/printoptionspage.ui"_ustr,
        u"modules/swriter/ui/renameautotextdialog.ui"_ustr,
        u"modules/swriter/ui/renameobjectdialog.ui"_ustr,
        u"modules/swriter/ui/rowheight.ui"_ustr,
        u"modules/swriter/ui/splittable.ui"_ustr,
        u"modules/swriter/ui/wordcount.ui"_ustr,
        u"sfx/ui/helpmanual.ui"_ustr,
        u"sfx/ui/inputdialog.ui"_ustr,
        u"sfx/ui/licensedialog.ui"_ustr,
        u"sfx/ui/loadtemplatedialog.ui"_ustr,
        u"sfx/ui/newstyle.ui"_ustr,
        u"sfx/ui/password.ui"_ustr,
        u"sfx/ui/printeroptionsdialog.ui"_ustr,
        u"sfx/ui/querysavedialog.ui"_ustr,
        u"sfx/ui/safemodequerydialog.ui"_ustr,
        u"sfx/ui/templatecategorydlg.ui"_ustr,
        u"sfx/ui/versioncommentdialog.ui"_ustr,
        u"sfx/ui/versionsofdialog.ui"_ustr,
        u"svt/ui/addresstemplatedialog.ui"_ustr,
        u"svt/ui/placeedit.ui"_ustr,
        u"svt/ui/printersetupdialog.ui"_ustr,
        u"svt/ui/restartdialog.ui"_ustr,
        u"svx/ui/compressgraphicdialog.ui"_ustr,
        u"svx/ui/docrecoverybrokendialog.ui"_ustr,
        u"svx/ui/docrecoveryrecoverdialog.ui"_ustr,
        u"svx/ui/docrecoverysavedialog.ui"_ustr,
        u"svx/ui/fontworkgallerydialog.ui"_ustr,
        u"svx/ui/deletefooterdialog.ui"_ustr,
        u"svx/ui/deleteheaderdialog.ui"_ustr,
        u"svx/ui/fileexporteddialog.ui"_ustr,
        u"svx/ui/gotopagedialog.ui"_ustr,
        u"svx/ui/safemodedialog.ui"_ustr,
        u"svx/ui/themecoloreditdialog.ui"_ustr,
        u"svx/ui/themedialog.ui"_ustr,
        u"uui/ui/password.ui"_ustr,
        u"vcl/ui/openlockedquerybox.ui"_ustr,
        u"vcl/ui/printerdevicepage.ui"_ustr,
        u"vcl/ui/printerpaperpage.ui"_ustr,
        u"vcl/ui/printerpropertiesdialog.ui"_ustr,
        u"vcl/ui/printprogressdialog.ui"_ustr,
        u"xmlsec/ui/digitalsignaturesdialog.ui"_ustr,
        u"xmlsec/ui/macrosecuritydialog.ui"_ustr,
        u"xmlsec/ui/securitylevelpage.ui"_ustr,
        u"xmlsec/ui/securitytrustpage.ui"_ustr,
        u"xmlsec/ui/selectcertificatedialog.ui"_ustr,
        u"xmlsec/ui/viewcertdialog.ui"_ustr,
        u"writerperfect/ui/exportepub.ui"_ustr,
    };

    // These UI files are only supported inside native Qt dialogs/widgets
    static std::unordered_set<OUString> aSupportedWithQtParent = {
        u"cui/ui/appearance.ui"_ustr,         u"cui/ui/graphictestentry.ui"_ustr,
        u"cui/ui/lineendstabpage.ui"_ustr,    u"cui/ui/linetabpage.ui"_ustr,
        u"cui/ui/linestyletabpage.ui"_ustr,   u"cui/ui/possizetabpage.ui"_ustr,
        u"cui/ui/rotationtabpage.ui"_ustr,    u"cui/ui/slantcornertabpage.ui"_ustr,
        u"cui/ui/textattrtabpage.ui"_ustr,    u"cui/ui/textanimtabpage.ui"_ustr,
        u"cui/ui/textcolumnstabpage.ui"_ustr, u"svt/ui/datewindow.ui"_ustr,
        u"svx/ui/colorwindow.ui"_ustr,        u"vcl/ui/editmenu.ui"_ustr,
        u"xmlsec/ui/certdetails.ui"_ustr,     u"xmlsec/ui/certgeneral.ui"_ustr,
    };

    if (aSupportedUIFiles.contains(rUIFile))
        return true;

    return aSupportedWithQtParent.contains(rUIFile)
           && dynamic_cast<const QtInstanceWidget*>(pParent);
}

std::unique_ptr<weld::MessageDialog> QtInstanceBuilder::weld_message_dialog(const OUString& id)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::MessageDialog> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QMessageBox* pMessageBox = m_xBuilder->get<QMessageBox>(id))
            xRet = std::make_unique<QtInstanceMessageDialog>(pMessageBox);
    });
    return xRet;
}

std::unique_ptr<weld::Dialog> QtInstanceBuilder::weld_dialog(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Dialog> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QDialog* pDialog = m_xBuilder->get<QDialog>(rId))
            xRet = std::make_unique<QtInstanceDialog>(pDialog);
    });
    return xRet;
}

std::unique_ptr<weld::Assistant> QtInstanceBuilder::weld_assistant(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Assistant> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QWizard* pWizard = m_xBuilder->get<QWizard>(rId))
            xRet = std::make_unique<QtInstanceAssistant>(pWizard);
    });
    return xRet;
}

std::unique_ptr<weld::Window> QtInstanceBuilder::create_screenshot_window()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Widget> QtInstanceBuilder::weld_widget(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Widget> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QWidget* pWidget = m_xBuilder->get<QWidget>(rId))
            xRet = std::make_unique<QtInstanceWidget>(pWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Container> QtInstanceBuilder::weld_container(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<QtInstanceContainer> xRet;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
        if (!pWidget)
            return;

        assert(pWidget->layout() && "no layout");
        xRet = std::make_unique<QtInstanceContainer>(pWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Box> QtInstanceBuilder::weld_box(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Box> xRet;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
        if (!pWidget)
            return;

        assert(qobject_cast<QBoxLayout*>(pWidget->layout()) && "widget doesn't have a box layout");
        xRet = std::make_unique<QtInstanceBox>(pWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Grid> QtInstanceBuilder::weld_grid(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Grid> xRet;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
        if (!pWidget)
            return;

        assert(qobject_cast<QGridLayout*>(pWidget->layout()) && "no grid layout");
        xRet = std::make_unique<QtInstanceGrid>(pWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Paned> QtInstanceBuilder::weld_paned(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Frame> QtInstanceBuilder::weld_frame(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Frame> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QGroupBox* pGroupBox = m_xBuilder->get<QGroupBox>(rId))
            xRet = std::make_unique<QtInstanceFrame>(pGroupBox);
    });
    return xRet;
}

std::unique_ptr<weld::ScrolledWindow> QtInstanceBuilder::weld_scrolled_window(const OUString& rId,
                                                                              bool)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::ScrolledWindow> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QScrollArea* pScrollArea = m_xBuilder->get<QScrollArea>(rId))
            xRet = std::make_unique<QtInstanceScrolledWindow>(pScrollArea);
    });
    return xRet;
}

std::unique_ptr<weld::Notebook> QtInstanceBuilder::weld_notebook(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Notebook> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QTabWidget* pTabWidget = m_xBuilder->get<QTabWidget>(rId))
            xRet = std::make_unique<QtInstanceNotebook>(pTabWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Button> QtInstanceBuilder::weld_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Button> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QPushButton* pButton = m_xBuilder->get<QPushButton>(rId))
            xRet = std::make_unique<QtInstanceButton>(pButton);
    });
    return xRet;
}

std::unique_ptr<weld::MenuButton> QtInstanceBuilder::weld_menu_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::MenuButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QToolButton* pButton = m_xBuilder->get<QToolButton>(rId))
            xRet = std::make_unique<QtInstanceMenuButton>(pButton);
    });
    return xRet;
}

std::unique_ptr<weld::LinkButton> QtInstanceBuilder::weld_link_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::LinkButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QtHyperlinkLabel* pLabel = m_xBuilder->get<QtHyperlinkLabel>(rId))
            xRet = std::make_unique<QtInstanceLinkButton>(pLabel);
    });
    return xRet;
}

std::unique_ptr<weld::ToggleButton> QtInstanceBuilder::weld_toggle_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::ToggleButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QAbstractButton* pButton = m_xBuilder->get<QAbstractButton>(rId))
            xRet = std::make_unique<QtInstanceToggleButton>(pButton);
    });
    return xRet;
}

std::unique_ptr<weld::RadioButton> QtInstanceBuilder::weld_radio_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::RadioButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QRadioButton* pRadioButton = m_xBuilder->get<QRadioButton>(rId))
            xRet = std::make_unique<QtInstanceRadioButton>(pRadioButton);
    });
    return xRet;
}

std::unique_ptr<weld::CheckButton> QtInstanceBuilder::weld_check_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::CheckButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QCheckBox* pCheckBox = m_xBuilder->get<QCheckBox>(rId))
            xRet = std::make_unique<QtInstanceCheckButton>(pCheckBox);
    });
    return xRet;
}

std::unique_ptr<weld::Scale> QtInstanceBuilder::weld_scale(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Scale> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QSlider* pSlider = m_xBuilder->get<QSlider>(rId))
            xRet = std::make_unique<QtInstanceScale>(pSlider);
    });
    return xRet;
}

std::unique_ptr<weld::ProgressBar> QtInstanceBuilder::weld_progress_bar(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::ProgressBar> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId))
            xRet = std::make_unique<QtInstanceProgressBar>(pProgressBar);
    });
    return xRet;
}

std::unique_ptr<weld::LevelBar> QtInstanceBuilder::weld_level_bar(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::LevelBar> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId))
            xRet = std::make_unique<QtInstanceLevelBar>(pProgressBar);
    });
    return xRet;
}

std::unique_ptr<weld::Spinner> QtInstanceBuilder::weld_spinner(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Spinner> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId))
            xRet = std::make_unique<QtInstanceSpinner>(pProgressBar);
    });
    return xRet;
}

std::unique_ptr<weld::Image> QtInstanceBuilder::weld_image(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Image> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QLabel* pLabel = m_xBuilder->get<QLabel>(rId))
            xRet = std::make_unique<QtInstanceImage>(pLabel);
    });
    return xRet;
}

std::unique_ptr<weld::Calendar> QtInstanceBuilder::weld_calendar(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Calendar> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QCalendarWidget* pCalendarWidget = m_xBuilder->get<QCalendarWidget>(rId))
            xRet = std::make_unique<QtInstanceCalendar>(pCalendarWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Entry> QtInstanceBuilder::weld_entry(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Entry> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QLineEdit* pLineEdit = m_xBuilder->get<QLineEdit>(rId))
            xRet = std::make_unique<QtInstanceEntry>(pLineEdit);
    });
    return xRet;
}

std::unique_ptr<weld::SpinButton> QtInstanceBuilder::weld_spin_button(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::SpinButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QtDoubleSpinBox* pSpinBox = m_xBuilder->get<QtDoubleSpinBox>(rId))
            xRet = std::make_unique<QtInstanceSpinButton>(pSpinBox);
    });
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
    SolarMutexGuard g;

    std::unique_ptr<weld::FormattedSpinButton> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QtDoubleSpinBox* pSpinBox = m_xBuilder->get<QtDoubleSpinBox>(rId))
            xRet = std::make_unique<QtInstanceFormattedSpinButton>(pSpinBox);
    });
    return xRet;
}

std::unique_ptr<weld::ComboBox> QtInstanceBuilder::weld_combo_box(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::ComboBox> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QComboBox* pComboBox = m_xBuilder->get<QComboBox>(rId))
            xRet = std::make_unique<QtInstanceComboBox>(pComboBox);
    });
    return xRet;
}

std::unique_ptr<weld::EntryTreeView>
QtInstanceBuilder::weld_entry_tree_view(const OUString& rContainerId, const OUString& rEntryId,
                                        const OUString& rTreeViewId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::EntryTreeView> xRet;
    GetQtInstance().RunInMainThread([&] {
        QWidget* pWidget = m_xBuilder->get<QWidget>(rContainerId);
        QLineEdit* pLineEdit = m_xBuilder->get<QLineEdit>(rEntryId);
        QTreeView* pTreeView = m_xBuilder->get<QTreeView>(rTreeViewId);
        assert(pWidget && pLineEdit && pTreeView);

        xRet = std::make_unique<QtInstanceEntryTreeView>(
            pWidget, pLineEdit, pTreeView, weld_entry(rEntryId), weld_tree_view(rTreeViewId));
    });

    return xRet;
}

std::unique_ptr<weld::TreeView> QtInstanceBuilder::weld_tree_view(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::TreeView> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QTreeView* pTreeView = m_xBuilder->get<QTreeView>(rId))
            xRet = std::make_unique<QtInstanceTreeView>(pTreeView);
    });
    return xRet;
}

std::unique_ptr<weld::IconView> QtInstanceBuilder::weld_icon_view(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::IconView> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QListView* pListView = m_xBuilder->get<QListView>(rId))
            xRet = std::make_unique<QtInstanceIconView>(pListView);
    });
    return xRet;
}

std::unique_ptr<weld::Label> QtInstanceBuilder::weld_label(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Label> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QLabel* pLabel = m_xBuilder->get<QLabel>(rId))
            xRet = std::make_unique<QtInstanceLabel>(pLabel);
    });
    return xRet;
}

std::unique_ptr<weld::TextView> QtInstanceBuilder::weld_text_view(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::TextView> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QPlainTextEdit* pTextEdit = m_xBuilder->get<QPlainTextEdit>(rId))
            xRet = std::make_unique<QtInstanceTextView>(pTextEdit);
    });
    return xRet;
}

std::unique_ptr<weld::Expander> QtInstanceBuilder::weld_expander(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Expander> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QtExpander* pExpander = m_xBuilder->get<QtExpander>(rId))
            xRet = std::make_unique<QtInstanceExpander>(pExpander);
    });
    return xRet;
}

std::unique_ptr<weld::DrawingArea>
QtInstanceBuilder::weld_drawing_area(const OUString& rId,
                                     const rtl::Reference<comphelper::OAccessible>& rA11yImpl,
                                     FactoryFunction, void*)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::DrawingArea> xRet;
    GetQtInstance().RunInMainThread([&] {
        QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
        if (!pLabel)
            return;

        xRet = std::make_unique<QtInstanceDrawingArea>(pLabel, rA11yImpl);
    });
    return xRet;
}

std::unique_ptr<weld::Menu> QtInstanceBuilder::weld_menu(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Menu> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QMenu* pMenu = m_xBuilder->get_menu(rId))
            xRet = std::make_unique<QtInstanceMenu>(pMenu);
    });
    return xRet;
}

std::unique_ptr<weld::Popover> QtInstanceBuilder::weld_popover(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Popover> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QWidget* pWidget = m_xBuilder->get<QWidget>(rId))
            xRet = std::make_unique<QtInstancePopover>(pWidget);
    });
    return xRet;
}

std::unique_ptr<weld::Toolbar> QtInstanceBuilder::weld_toolbar(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Toolbar> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QToolBar* pToolBar = m_xBuilder->get<QToolBar>(rId))
            xRet = std::make_unique<QtInstanceToolbar>(pToolBar);
    });
    return xRet;
}

std::unique_ptr<weld::Scrollbar> QtInstanceBuilder::weld_scrollbar(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Scrollbar> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QScrollBar* pScrollBar = m_xBuilder->get<QScrollBar>(rId))
            xRet = std::make_unique<QtInstanceScrollbar>(pScrollBar);
    });
    return xRet;
}

std::unique_ptr<weld::SizeGroup> QtInstanceBuilder::create_size_group()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
