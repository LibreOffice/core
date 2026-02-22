/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceBuilder.hxx>

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
#include <QtInstancePaned.hxx>
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
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_set.h>

#include <string_view>

// set of supported UI files
//
// The idea is to implement functionality needed for a specific UI file/dialog
// in QtInstanceBuilder, then add it to the set of supported UI files here.
// This allows looking at one .ui file at a time and only having to implement
// what is relevant for that particular one, without having to implement the full
// weld API at once.
constexpr auto SUPPORTED_UI_FILES = frozen::make_unordered_set<std::u16string_view>({
    u"cui/ui/aboutconfigdialog.ui",
    u"cui/ui/aboutdialog.ui",
    u"cui/ui/additionsdialog.ui",
    u"cui/ui/breaknumberoption.ui",
    u"cui/ui/certdialog.ui",
    u"cui/ui/comment.ui",
    u"cui/ui/editdictionarydialog.ui",
    u"cui/ui/eventassigndialog.ui",
    u"cui/ui/fontfeaturesdialog.ui",
    u"cui/ui/fontfragment.ui",
    u"cui/ui/gallerythemedialog.ui",
    u"cui/ui/gallerytitledialog.ui",
    u"cui/ui/graphictestdlg.ui",
    u"cui/ui/hyperlinkdlg.ui",
    u"cui/ui/hyperlinkmarkdialog.ui",
    u"cui/ui/imageviewer.ui",
    u"cui/ui/insertoleobject.ui",
    u"cui/ui/insertrowcolumn.ui",
    u"cui/ui/javaclasspathdialog.ui",
    u"cui/ui/javastartparametersdialog.ui",
    u"cui/ui/linedialog.ui",
    u"cui/ui/macroassigndialog.ui",
    u"cui/ui/macroselectordialog.ui",
    u"cui/ui/namedialog.ui",
    u"cui/ui/newtabledialog.ui",
    u"cui/ui/numberdialog.ui",
    u"cui/ui/formatnumberdialog.ui",
    u"cui/ui/objecttitledescdialog.ui",
    u"cui/ui/objectnamedialog.ui",
    u"cui/ui/optnewdictionarydialog.ui",
    u"cui/ui/password.ui",
    u"cui/ui/pastespecial.ui",
    u"cui/ui/positionsizedialog.ui",
    u"cui/ui/qrcodegen.ui",
    u"cui/ui/querydeletechartcolordialog.ui",
    u"cui/ui/querydeletedictionarydialog.ui",
    u"cui/ui/querydeletelineenddialog.ui",
    u"cui/ui/querydeletelinestyledialog.ui",
    u"cui/ui/querydialog.ui",
    u"cui/ui/queryduplicatedialog.ui",
    u"cui/ui/querysavelistdialog.ui",
    u"cui/ui/queryupdategalleryfilelistdialog.ui",
    u"cui/ui/screenshotannotationdialog.ui",
    u"cui/ui/searchattrdialog.ui",
    u"cui/ui/securityoptionsdialog.ui",
    u"cui/ui/selectpathdialog.ui",
    u"cui/ui/signatureline.ui",
    u"cui/ui/signsignatureline.ui",
    u"cui/ui/similaritysearchdialog.ui",
    u"cui/ui/spelloptionsdialog.ui",
    u"cui/ui/splitcellsdialog.ui",
    u"cui/ui/textdialog.ui",
    u"cui/ui/tipofthedaydialog.ui",
    u"cui/ui/toolbartabpage.ui",
    u"cui/ui/tsaurldialog.ui",
    u"cui/ui/uipickerdialog.ui",
    u"cui/ui/uitabpage.ui",
    u"cui/ui/welcomedialog.ui",
    u"cui/ui/whatsnewtabpage.ui",
    u"cui/ui/widgettestdialog.ui",
    u"cui/ui/zoomdialog.ui",
    u"dbaccess/ui/designsavemodifieddialog.ui",
    u"dbaccess/ui/fielddialog.ui",
    u"dbaccess/ui/queryfilterdialog.ui",
    u"dbaccess/ui/savedialog.ui",
    u"dbaccess/ui/sqlexception.ui",
    u"dbaccess/ui/tabledesignsavemodifieddialog.ui",
    u"desktop/ui/installforalldialog.ui",
    u"desktop/ui/showlicensedialog.ui",
    u"desktop/ui/licensedialog.ui",
    u"filter/ui/pdfoptionsdialog.ui",
    u"filter/ui/testxmlfilter.ui",
    u"filter/ui/xmlfiltertabpagegeneral.ui",
    u"filter/ui/xmlfiltertabpagetransformation.ui",
    u"filter/ui/xmlfiltersettings.ui",
    u"filter/ui/xsltfilterdialog.ui",
    u"fps/ui/foldernamedialog.ui",
    u"modules/BasicIDE/ui/colorscheme.ui",
    u"modules/BasicIDE/ui/defaultlanguage.ui",
    u"modules/BasicIDE/ui/deletelangdialog.ui",
    u"modules/BasicIDE/ui/exportdialog.ui",
    u"modules/BasicIDE/ui/gotolinedialog.ui",
    u"modules/BasicIDE/ui/managelanguages.ui",
    u"modules/BasicIDE/ui/newlibdialog.ui",
    u"modules/sbibliography/ui/choosedatasourcedialog.ui",
    u"modules/sbibliography/ui/mappingdialog.ui",
    u"modules/scalc/ui/colwidthdialog.ui",
    u"modules/scalc/ui/condformatmanager.ui",
    u"modules/scalc/ui/conditionalformatdialog.ui",
    u"modules/scalc/ui/definedatabaserangedialog.ui",
    u"modules/scalc/ui/definetablerangedialog.ui",
    u"modules/scalc/ui/deletecells.ui",
    u"modules/scalc/ui/deletecontents.ui",
    u"modules/scalc/ui/duplicaterecordsdlg.ui",
    u"modules/scalc/ui/fourieranalysisdialog.ui",
    u"modules/scalc/ui/gotosheetdialog.ui",
    u"modules/scalc/ui/groupbydate.ui",
    u"modules/scalc/ui/groupdialog.ui",
    u"modules/scalc/ui/inputstringdialog.ui",
    u"modules/scalc/ui/insertcells.ui",
    u"modules/scalc/ui/insertsheet.ui",
    u"modules/scalc/ui/movecopysheet.ui",
    u"modules/scalc/ui/optdlg.ui",
    u"modules/scalc/ui/optimalcolwidthdialog.ui",
    u"modules/scalc/ui/optimalrowheightdialog.ui",
    u"modules/scalc/ui/pastespecial.ui",
    u"modules/scalc/ui/rowheightdialog.ui",
    u"modules/scalc/ui/scenariodialog.ui",
    u"modules/scalc/ui/selectrange.ui",
    u"modules/scalc/ui/selectsource.ui",
    u"modules/scalc/ui/sharedocumentdlg.ui",
    u"modules/scalc/ui/showsheetdialog.ui",
    u"modules/scalc/ui/sortdialog.ui",
    u"modules/scalc/ui/subtotaldialog.ui",
    u"modules/scalc/ui/ttestdialog.ui",
    u"modules/scalc/ui/ztestdialog.ui",
    u"modules/schart/ui/charttypedialog.ui",
    u"modules/schart/ui/dlg_DataLabel.ui",
    u"modules/schart/ui/dlg_InsertDataTable.ui",
    u"modules/schart/ui/dlg_InsertErrorBars.ui",
    u"modules/schart/ui/dlg_InsertLegend.ui",
    u"modules/schart/ui/insertaxisdlg.ui",
    u"modules/schart/ui/insertgriddlg.ui",
    u"modules/schart/ui/inserttitledlg.ui",
    u"modules/sdraw/ui/dlgsnap.ui",
    u"modules/sdraw/ui/insertlayer.ui",
    u"modules/simpress/ui/customslideshows.ui",
    u"modules/simpress/ui/definecustomslideshow.ui",
    u"modules/simpress/ui/interactiondialog.ui",
    u"modules/simpress/ui/masterlayoutdlg.ui",
    u"modules/simpress/ui/presentationdialog.ui",
    u"modules/simpress/ui/slidedesigndialog.ui",
    u"modules/smath/ui/alignmentdialog.ui",
    u"modules/smath/ui/fontdialog.ui",
    u"modules/smath/ui/fontsizedialog.ui",
    u"modules/smath/ui/fonttypedialog.ui",
    u"modules/smath/ui/savedefaultsdialog.ui",
    u"modules/smath/ui/smathsettings.ui",
    u"modules/smath/ui/spacingdialog.ui",
    u"modules/spropctrlr/ui/taborder.ui",
    u"modules/swriter/ui/addentrydialog.ui",
    u"modules/swriter/ui/assignfieldsdialog.ui",
    u"modules/swriter/ui/authenticationsettingsdialog.ui",
    u"modules/swriter/ui/autoformattable.ui",
    u"modules/swriter/ui/bibliographyentry.ui",
    u"modules/swriter/ui/captiondialog.ui",
    u"modules/swriter/ui/captionoptions.ui",
    u"modules/swriter/ui/ccdialog.ui",
    u"modules/swriter/ui/columndialog.ui",
    u"modules/swriter/ui/columnwidth.ui",
    u"modules/swriter/ui/createaddresslist.ui",
    u"modules/swriter/ui/createauthorentry.ui",
    u"modules/swriter/ui/customizeaddrlistdialog.ui",
    u"modules/swriter/ui/editcategories.ui",
    u"modules/swriter/ui/endnotepage.ui",
    u"modules/swriter/ui/exchangedatabases.ui",
    u"modules/swriter/ui/fielddialog.ui",
    u"modules/swriter/ui/findentrydialog.ui",
    u"modules/swriter/ui/footendnotedialog.ui",
    u"modules/swriter/ui/footnotepage.ui",
    u"modules/swriter/ui/indexentry.ui",
    u"modules/swriter/ui/inforeadonlydialog.ui",
    u"modules/swriter/ui/insertbookmark.ui",
    u"modules/swriter/ui/insertbreak.ui",
    u"modules/swriter/ui/insertcaption.ui",
    u"modules/swriter/ui/inserttable.ui",
    u"modules/swriter/ui/labeldialog.ui",
    u"modules/swriter/ui/linenumbering.ui",
    u"modules/swriter/ui/mmcreatingdialog.ui",
    u"modules/swriter/ui/mmresultprintdialog.ui",
    u"modules/swriter/ui/mmresultsavedialog.ui",
    u"modules/swriter/ui/newuserindexdialog.ui",
    u"modules/swriter/ui/numberingnamedialog.ui",
    u"modules/swriter/ui/outlinenumbering.ui",
    u"modules/swriter/ui/pagenumberdlg.ui",
    u"modules/swriter/ui/printmergedialog.ui",
    u"modules/swriter/ui/renameautotextdialog.ui",
    u"modules/swriter/ui/renameentrydialog.ui",
    u"modules/swriter/ui/renameobjectdialog.ui",
    u"modules/swriter/ui/rowheight.ui",
    u"modules/swriter/ui/savelabeldialog.ui",
    u"modules/swriter/ui/splittable.ui",
    u"modules/swriter/ui/stringinput.ui",
    u"modules/swriter/ui/testmailsettings.ui",
    u"modules/swriter/ui/watermarkdialog.ui",
    u"modules/swriter/ui/wordcount.ui",
    u"modules/swriter/ui/wrapdialog.ui",
    u"sfx/ui/commandpopup.ui",
    u"sfx/ui/documentpropertiesdialog.ui",
    u"sfx/ui/editdurationdialog.ui",
    u"sfx/ui/helpmanual.ui",
    u"sfx/ui/inputdialog.ui",
    u"sfx/ui/licensedialog.ui",
    u"sfx/ui/loadtemplatedialog.ui",
    u"sfx/ui/newstyle.ui",
    u"sfx/ui/password.ui",
    u"sfx/ui/printeroptionsdialog.ui",
    u"sfx/ui/querysavedialog.ui",
    u"sfx/ui/safemodequerydialog.ui",
    u"sfx/ui/saveastemplatedlg.ui",
    u"sfx/ui/templatecategorydlg.ui",
    u"sfx/ui/versioncommentdialog.ui",
    u"sfx/ui/versionsofdialog.ui",
    u"svt/ui/addresstemplatedialog.ui",
    u"svt/ui/placeedit.ui",
    u"svt/ui/printersetupdialog.ui",
    u"svt/ui/restartdialog.ui",
    u"svx/ui/compressgraphicdialog.ui",
    u"svx/ui/docrecoverybrokendialog.ui",
    u"svx/ui/docrecoveryrecoverdialog.ui",
    u"svx/ui/docrecoverysavedialog.ui",
    u"svx/ui/dropdownfielddialog.ui",
    u"svx/ui/fontworkgallerydialog.ui",
    u"svx/ui/deletefooterdialog.ui",
    u"svx/ui/deleteheaderdialog.ui",
    u"svx/ui/fileexporteddialog.ui",
    u"svx/ui/formpropertydialog.ui",
    u"svx/ui/gotopagedialog.ui",
    u"svx/ui/linkwarndialog.ui",
    u"svx/ui/passwd.ui",
    u"svx/ui/querydeleteobjectdialog.ui",
    u"svx/ui/querydeletethemedialog.ui",
    u"svx/ui/safemodedialog.ui",
    u"svx/ui/themecoloreditdialog.ui",
    u"svx/ui/themedialog.ui",
    u"uui/ui/logindialog.ui",
    u"uui/ui/masterpassworddlg.ui",
    u"uui/ui/password.ui",
    u"uui/ui/setmasterpassworddlg.ui",
    u"vcl/ui/openlockedquerybox.ui",
    u"vcl/ui/printdialog.ui",
    u"vcl/ui/printerdevicepage.ui",
    u"vcl/ui/printerpaperpage.ui",
    u"vcl/ui/printerpropertiesdialog.ui",
    u"vcl/ui/printprogressdialog.ui",
    u"xmlsec/ui/digitalsignaturesdialog.ui",
    u"xmlsec/ui/macrosecuritydialog.ui",
    u"xmlsec/ui/securitylevelpage.ui",
    u"xmlsec/ui/securitytrustpage.ui",
    u"xmlsec/ui/selectcertificatedialog.ui",
    u"xmlsec/ui/viewcertdialog.ui",
    u"writerperfect/ui/exportepub.ui",
    u"writerperfect/ui/wpftencodingdialog.ui",
});

// These UI files are only supported inside native Qt dialogs/widgets
constexpr auto SUPPORTED_WITH_QT_PARENT = frozen::make_unordered_set<std::u16string_view>({
    u"cui/ui/additionsfragment.ui",
    u"cui/ui/appearance.ui",
    u"cui/ui/cellalignment.ui",
    u"cui/ui/eventassignpage.ui",
    u"cui/ui/galleryfilespage.ui",
    u"cui/ui/gallerygeneralpage.ui",
    u"cui/ui/graphictestentry.ui",
    u"cui/ui/hyperlinkdocpage.ui",
    u"cui/ui/hyperlinkinternetpage.ui",
    u"cui/ui/hyperlinkmailpage.ui",
    u"cui/ui/hyperlinknewdocpage.ui",
    u"cui/ui/lineendstabpage.ui",
    u"cui/ui/linetabpage.ui",
    u"cui/ui/linestyletabpage.ui",
    u"cui/ui/macroassignpage.ui",
    u"cui/ui/numberingformatpage.ui",
    u"cui/ui/optlingupage.ui",
    u"cui/ui/possizetabpage.ui",
    u"cui/ui/rotationtabpage.ui",
    u"cui/ui/spinbox.ui",
    u"cui/ui/slantcornertabpage.ui",
    u"cui/ui/textattrtabpage.ui",
    u"cui/ui/textanimtabpage.ui",
    u"cui/ui/textcolumnstabpage.ui",
    u"filter/ui/pdfgeneralpage.ui",
    u"filter/ui/pdflinkspage.ui",
    u"filter/ui/pdfsecuritypage.ui",
    u"filter/ui/pdfsignpage.ui",
    u"filter/ui/pdfuserinterfacepage.ui",
    u"filter/ui/pdfviewpage.ui",
    u"modules/scalc/ui/conditionalentry.ui",
    u"modules/scalc/ui/printeroptions.ui",
    u"modules/scalc/ui/sortcriteriapage.ui",
    u"modules/scalc/ui/sortkey.ui",
    u"modules/scalc/ui/sortoptionspage.ui",
    u"modules/scalc/ui/statisticsinfopage.ui",
    u"modules/scalc/ui/subtotalgrppage.ui",
    u"modules/scalc/ui/subtotaloptionspage.ui",
    u"modules/schart/ui/tp_ChartType.ui",
    u"modules/sdraw/ui/drawprinteroptions.ui",
    u"modules/simpress/ui/annotationtagmenu.ui",
    u"modules/simpress/ui/impressprinteroptions.ui",
    u"modules/simpress/ui/interactionpage.ui",
    u"modules/smath/ui/printeroptions.ui",
    u"modules/spropctrlr/ui/browserline.ui",
    u"modules/spropctrlr/ui/browserpage.ui",
    u"modules/spropctrlr/ui/combobox.ui",
    u"modules/spropctrlr/ui/formproperties.ui",
    u"modules/spropctrlr/ui/listbox.ui",
    u"modules/spropctrlr/ui/numericfield.ui",
    u"modules/spropctrlr/ui/textfield.ui",
    u"modules/spropctrlr/ui/urlcontrol.ui",
    u"modules/swriter/ui/addressfragment.ui",
    u"modules/swriter/ui/assignfragment.ui",
    u"modules/swriter/ui/bibliofragment.ui",
    u"modules/swriter/ui/businessdatapage.ui",
    u"modules/swriter/ui/cardmediumpage.ui",
    u"modules/swriter/ui/columnpage.ui",
    u"modules/swriter/ui/flddbpage.ui",
    u"modules/swriter/ui/flddocinfopage.ui",
    u"modules/swriter/ui/flddocumentpage.ui",
    u"modules/swriter/ui/fldfuncpage.ui",
    u"modules/swriter/ui/fldrefpage.ui",
    u"modules/swriter/ui/fldvarpage.ui",
    u"modules/swriter/ui/labelformatpage.ui",
    u"modules/swriter/ui/labeloptionspage.ui",
    u"modules/swriter/ui/optcaptionpage.ui",
    u"modules/swriter/ui/outlinenumberingpage.ui",
    u"modules/swriter/ui/outlinepositionpage.ui",
    u"modules/swriter/ui/printeroptions.ui",
    u"modules/swriter/ui/printoptionspage.ui",
    u"modules/swriter/ui/privateuserpage.ui",
    u"modules/swriter/ui/statisticsinfopage.ui",
    u"modules/swriter/ui/wrappage.ui",
    u"sfx/ui/custominfopage.ui",
    u"sfx/ui/descriptioninfopage.ui",
    u"sfx/ui/documentfontspage.ui",
    u"sfx/ui/documentinfopage.ui",
    u"sfx/ui/linefragment.ui",
    u"sfx/ui/securityinfopage.ui",
    u"svt/ui/datewindow.ui",
    u"svt/ui/linewindow.ui",
    u"svx/ui/colorwindow.ui",
    u"svx/ui/dockingfontwork.ui",
    u"svx/ui/formnavigator.ui",
    u"svx/ui/formnavimenu.ui",
    u"vcl/ui/editmenu.ui",
    u"vcl/ui/screenshotmenu.ui",
    u"xmlsec/ui/certdetails.ui",
    u"xmlsec/ui/certgeneral.ui",
});

QtInstanceBuilder::QtInstanceBuilder(QWidget* pParent, std::u16string_view sUIRoot,
                                     const OUString& rUIFile)
    : m_xBuilder(std::make_unique<QtBuilder>(pParent, sUIRoot, rUIFile))
{
}

QtInstanceBuilder::~QtInstanceBuilder() {}

bool QtInstanceBuilder::IsUIFileSupported(const OUString& rUIFile, const weld::Widget* pParent)
{
    if (SUPPORTED_UI_FILES.contains(rUIFile))
        return true;

    return SUPPORTED_WITH_QT_PARENT.contains(rUIFile)
           && dynamic_cast<const QtInstanceWidget*>(pParent);
}

bool QtInstanceBuilder::IsInterimUIFileSupported(const OUString& rUIFile)
{
    return SUPPORTED_WITH_QT_PARENT.contains(rUIFile);
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
    return weld_dialog(m_xBuilder->getDialogId());
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

std::unique_ptr<weld::Paned> QtInstanceBuilder::weld_paned(const OUString& rId)
{
    SolarMutexGuard g;

    std::unique_ptr<weld::Paned> xRet;
    GetQtInstance().RunInMainThread([&] {
        if (QSplitter* pSplitter = m_xBuilder->get<QSplitter>(rId))
            xRet = std::make_unique<QtInstancePaned>(pSplitter);
    });
    return xRet;
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
