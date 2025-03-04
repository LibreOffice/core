/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/string.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>
#include <jsdialog/enabled.hxx>
#include <vector>

namespace
{
enum JSDialogEnabledType
{
    Ignore = 0,
    Dialog = 1,
    Popup = 2,
    Menu = 3,
    Sidebar = 4,
    Notebookbar = 5,
    AddressInput = 6,
    Formulabar = 7,
    MobileDialog = 8,
};

// ========== IGNORED ======================================================= //

constexpr auto IgnoredList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        { u"sfx/ui/deck.ui", JSDialogEnabledType::Ignore },
        { u"sfx/ui/tabbar.ui", JSDialogEnabledType::Ignore },
        { u"sfx/ui/tabbarcontents.ui", JSDialogEnabledType::Ignore },
        { u"svt/ui/scrollbars.ui", JSDialogEnabledType::Ignore },
        { u"svx/ui/selectionmenu.ui", JSDialogEnabledType::Ignore },
        { u"svx/ui/stylemenu.ui", JSDialogEnabledType::Ignore },
        { u"svt/ui/tabbuttons.ui", JSDialogEnabledType::Ignore },
        { u"svx/ui/toolbarpopover.ui", JSDialogEnabledType::Ignore }
    });

// ========== MOBILE DIALOGS ================================================= //

constexpr auto MobileDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        { u"modules/swriter/ui/watermarkdialog.ui", JSDialogEnabledType::MobileDialog },
        { u"modules/swriter/ui/wordcount-mobile.ui", JSDialogEnabledType::MobileDialog },
        { u"svx/ui/findreplacedialog-mobile.ui", JSDialogEnabledType::MobileDialog }
    });

// ========== DIALOGS ======================================================= //
// Split into few - to allow constexpr and manage order

constexpr auto CuiDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // cui
        { u"cui/ui/areatabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/areadialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/asiantypography.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/borderpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/bulletandposition.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/cellalignment.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/charnamepage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/colorpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/colorpickerdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/croppage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/effectspage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/eventassigndialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/fontfeaturesdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/formatcellsdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/formatnumberdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/gradientpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/hatchpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/hyperlinkdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/hyperlinkinternetpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/hyperlinkmailpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/imagetabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/linedialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/lineendstabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/linestyletabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/linetabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/macroselectordialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/namedialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/numberingformatpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/numberingoptionspage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/numberingpositionpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/objectnamedialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/objecttitledescdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/optlingupage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/pageformatpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/paragalignpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/paraindentspacing.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/paratabspage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/password.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/pastespecial.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/patterntabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/pickbulletpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/pickgraphicpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/picknumberingpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/pickoutlinepage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/positionpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/positionsizedialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/possizetabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/rotationtabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/shadowtabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/slantcornertabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/spinbox.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/queryduplicatedialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/similaritysearchdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/specialcharacters.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/spellingdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/spelloptionsdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/splitcellsdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/textflowpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/thesaurus.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/transparencytabpage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/twolinespage.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/widgettestdialog.ui", JSDialogEnabledType::Dialog },
        { u"cui/ui/qrcodegen.ui", JSDialogEnabledType::Dialog }
    });

constexpr auto SfxDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // sfx
        { u"sfx/ui/cmisinfopage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/custominfopage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/descriptioninfopage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/documentinfopage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/documentfontspage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/documentpropertiesdialog.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/editdurationdialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/headfootformatpage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/linefragment.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/managestylepage.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/newstyle.ui", JSDialogEnabledType::Dialog },
        { u"sfx/ui/password.ui", JSDialogEnabledType::Dialog }
    });

constexpr auto ScalcDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // scalc
        { u"modules/scalc/ui/advancedfilterdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/analysisofvariancedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/cellprotectionpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/chardialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/chisquaretestdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/colwidthdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/conditionaleasydialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/condformatmanager.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/correlationdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/covariancedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/datafielddialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/datafieldoptionsdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/definename.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/deletecells.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/deletecontents.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/descriptivestatisticsdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/erroralerttabpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/exponentialsmoothingdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/formatcellsdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/fourieranalysisdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/goalseekdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/groupdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/headerfootercontent.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/headerfooterdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/imoptdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/insertcells.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/managenamesdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/movecopysheet.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/movingaveragedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/optimalcolwidthdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/optimalrowheightdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/pagetemplatedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/paratemplatedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/pastespecial.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/pivotfielddialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/pivottablelayoutdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/protectsheetdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/regressiondialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/rowheightdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/samplingdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/selectsource.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/sheetprintpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/simplerefdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/sortcriteriapage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/sortdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/sortkey.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/sortoptionspage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/sparklinedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/standardfilterdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/textimportcsv.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/ttestdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/ungroupdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/validationcriteriapage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/validationdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/validationhelptabpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/warnautocorrect.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/ztestdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/definedatabaserangedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/scalc/ui/selectrange.ui", JSDialogEnabledType::Dialog }
    });

constexpr auto SwriterDialogList
= frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // swriter
        { u"modules/swriter/ui/bulletsandnumbering.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/captionoptions.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/characterproperties.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/charurlpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/columndialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/columnpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/contentcontroldlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/contentcontrollistitemdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/dropcapspage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/dropdownfielddialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/editsectiondialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/endnotepage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/footendnotedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/footnoteareapage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/footnotepage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/footnotesendnotestabpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/formatsectiondialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/formattablepage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/framedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/frmaddpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/frmurlpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/frmtypepage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/indentpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/indexentry.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/inforeadonlydialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/insertbreak.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/insertcaption.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/insertsectiondialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/linenumbering.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/newuserindexdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/numparapage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/objectdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/pagenumberdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/paradialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/picturedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/picturepage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/sectionpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/sortdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/splittable.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tablecolumnpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tableproperties.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tabletextflowpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/templatedialog1.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/templatedialog2.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/templatedialog8.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/textgridpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/titlepage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tocdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tocentriespage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tocindexpage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/tocstylespage.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/translationdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/watermarkdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/wordcount.ui", JSDialogEnabledType::Dialog },
        { u"modules/swriter/ui/wrappage.ui", JSDialogEnabledType::Dialog }
});

constexpr auto SimpressDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // simpress
        { u"modules/simpress/ui/customanimationeffecttab.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/customanimationproperties.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/customanimationtexttab.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/customanimationtimingtab.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/headerfooterdialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/headerfootertab.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/interactiondialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/simpress/ui/interactionpage.ui", JSDialogEnabledType::Dialog }
    });

constexpr auto SdrawDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
    // sdraw
    { u"modules/sdraw/ui/drawchardialog.ui", JSDialogEnabledType::Dialog },
    { u"modules/sdraw/ui/drawpagedialog.ui", JSDialogEnabledType::Dialog },
    { u"modules/sdraw/ui/drawparadialog.ui", JSDialogEnabledType::Dialog }
    });

constexpr auto SchartDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // schart
        { u"modules/schart/ui/attributedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/charttypedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/datarangedialog.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/insertaxisdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/inserttitledlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/smoothlinesdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/steppedlinesdlg.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/tp_ChartType.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/tp_DataSource.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/tp_RangeChooser.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/tp_Trendline.ui", JSDialogEnabledType::Dialog },
        { u"modules/schart/ui/wizelementspage.ui", JSDialogEnabledType::Dialog }
    });

constexpr auto OtherDialogList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // formula
        { u"formula/ui/formuladialog.ui", JSDialogEnabledType::Dialog },
        { u"formula/ui/functionpage.ui", JSDialogEnabledType::Dialog },
        { u"formula/ui/parameter.ui", JSDialogEnabledType::Dialog },
        { u"formula/ui/structpage.ui", JSDialogEnabledType::Dialog },
        // svx
        { u"svx/ui/acceptrejectchangesdialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/accessibilitycheckdialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/accessibilitycheckentry.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/compressgraphicdialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/findreplacedialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/fontworkgallerydialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/headfootformatpage.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/redlinecontrol.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/redlinefilterpage.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/redlineviewpage.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/themecoloreditdialog.ui", JSDialogEnabledType::Dialog },
        { u"svx/ui/themedialog.ui", JSDialogEnabledType::Dialog },
        // uui
        { u"uui/ui/logindialog.ui", JSDialogEnabledType::Dialog },
        { u"uui/ui/macrowarnmedium.ui", JSDialogEnabledType::Dialog },
        // vcl
        { u"vcl/ui/wizard.ui", JSDialogEnabledType::Dialog },
        // filter
        { u"filter/ui/pdfgeneralpage.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/pdflinkspage.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/pdfoptionsdialog.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/pdfsecuritypage.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/pdfsignpage.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/pdfuserinterfacepage.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/pdfviewpage.ui", JSDialogEnabledType::Dialog },
        { u"filter/ui/warnpdfdialog.ui", JSDialogEnabledType::Dialog },
        // writerperfect
        { u"writerperfect/ui/exportepub.ui", JSDialogEnabledType::Dialog },
        { u"writerperfect/ui/wpftencodingdialog.ui", JSDialogEnabledType::Dialog },
        // xmlsec
        { u"xmlsec/ui/certgeneral.ui", JSDialogEnabledType::Dialog },
        { u"xmlsec/ui/certpage.ui", JSDialogEnabledType::Dialog },
        { u"xmlsec/ui/digitalsignaturesdialog.ui", JSDialogEnabledType::Dialog },
        { u"xmlsec/ui/selectcertificatedialog.ui", JSDialogEnabledType::Dialog },
        { u"xmlsec/ui/viewcertdialog.ui", JSDialogEnabledType::Dialog }
    });

// ========== POPUP ====================================================== //

constexpr auto PopupList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // scalc
        { u"modules/scalc/ui/filterdropdown.ui", JSDialogEnabledType::Popup },
        { u"modules/scalc/ui/filterlist.ui", JSDialogEnabledType::Popup },
        { u"modules/scalc/ui/filtersubdropdown.ui", JSDialogEnabledType::Popup },
        { u"modules/scalc/ui/floatingborderstyle.ui", JSDialogEnabledType::Popup },
        { u"modules/scalc/ui/floatinglinestyle.ui", JSDialogEnabledType::Popup },
        // svt
        { u"svt/ui/datewindow.ui", JSDialogEnabledType::Popup },
        { u"svt/ui/linewindow.ui", JSDialogEnabledType::Popup },
        // svx
        { u"svx/ui/colorwindow.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/currencywindow.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/floatingareastyle.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/floatingframeborder.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/floatinglineend.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/floatinglineproperty.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/floatinglinestyle.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/fontworkalignmentcontrol.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/fontworkcharacterspacingcontrol.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/numberingwindow.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/paralinespacingcontrol.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/textcharacterspacingcontrol.ui", JSDialogEnabledType::Popup },
        { u"svx/ui/textunderlinecontrol.ui", JSDialogEnabledType::Popup }
    });

// ========== MENU ======================================================= //

constexpr auto MenuList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        { u"sfx/ui/stylecontextmenu.ui", JSDialogEnabledType::Menu }
    });

// ========== SIDEBAR ==================================================== //

constexpr auto SidebarList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        // scalc
        { u"modules/scalc/ui/functionpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/scalc/ui/navigatorpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/scalc/ui/sidebaralignment.ui", JSDialogEnabledType::Sidebar },
        { u"modules/scalc/ui/sidebarcellappearance.ui", JSDialogEnabledType::Sidebar },
        { u"modules/scalc/ui/sidebarnumberformat.ui", JSDialogEnabledType::Sidebar },
        // schart
        { u"modules/schart/ui/sidebaraxis.ui", JSDialogEnabledType::Sidebar },
        { u"modules/schart/ui/sidebarelements.ui", JSDialogEnabledType::Sidebar },
        { u"modules/schart/ui/sidebarerrorbar.ui", JSDialogEnabledType::Sidebar },
        { u"modules/schart/ui/sidebarseries.ui", JSDialogEnabledType::Sidebar },
        { u"modules/schart/ui/sidebartype.ui", JSDialogEnabledType::Sidebar },
        // simpress
        { u"modules/simpress/ui/customanimationfragment.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/customanimationspanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/layoutpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/masterpagepanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/masterpagepanelall.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/masterpagepanelrecent.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/navigatorpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/sidebarslidebackground.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/slidetransitionspanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/simpress/ui/tabledesignpanel.ui", JSDialogEnabledType::Sidebar },
        // smath
        { u"modules/smath/ui/sidebarelements_math.ui", JSDialogEnabledType::Sidebar },
        { u"modules/smath/ui/sidebarproperties_math.ui", JSDialogEnabledType::Sidebar },
        // swriter
        { u"modules/swriter/ui/a11ycheckissuespanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/managechangessidebar.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/navigatorpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/pagefooterpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/pageformatpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/pageheaderpanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/pagestylespanel.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/sidebarstylepresets.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/sidebartableedit.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/sidebartheme.ui", JSDialogEnabledType::Sidebar },
        { u"modules/swriter/ui/sidebarwrap.ui", JSDialogEnabledType::Sidebar },
        // sfx
        { u"sfx/ui/panel.ui", JSDialogEnabledType::Sidebar },
        { u"sfx/ui/templatepanel.ui", JSDialogEnabledType::Sidebar },
        // svx
        { u"svx/ui/defaultshapespanel.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/inspectortextpanel.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/mediaplayback.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebararea.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebareffect.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarempty.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarfontwork.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebargallery.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebargraphic.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarline.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarlists.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarparagraph.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarpossize.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarshadow.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebarstylespanel.ui", JSDialogEnabledType::Sidebar },
        { u"svx/ui/sidebartextpanel.ui", JSDialogEnabledType::Sidebar }
    });

// ========== NOTEBOOKBAR ================================================= //

constexpr auto NotebookbarList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        { u"modules/scalc/ui/numberbox.ui", JSDialogEnabledType::Notebookbar },
        { u"svx/ui/fontnamebox.ui", JSDialogEnabledType::Notebookbar },
        { u"svx/ui/fontsizebox.ui", JSDialogEnabledType::Notebookbar },
        { u"svx/ui/stylespreview.ui", JSDialogEnabledType::Notebookbar }
    });

// ========== ADDRESSINPUT ================================================ //

constexpr auto AddressInputList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        { u"modules/scalc/ui/posbox.ui", JSDialogEnabledType::AddressInput }
    });

// ========== FORMULABAR ================================================== //

constexpr auto FormulabarList
    = frozen::make_unordered_map<std::u16string_view, JSDialogEnabledType>({
        { u"modules/scalc/ui/inputbar.ui", JSDialogEnabledType::Formulabar }
    });

// ========== LOOKUP ====================================================== //

inline bool isInMap(const auto& rList, std::u16string_view rUIFile, JSDialogEnabledType eType)
{
    auto aFound = rList.find(rUIFile);
    if (aFound != rList.end())
    {
        bool bIsMatchingType = aFound->second == eType;
        assert(bIsMatchingType);
        return bIsMatchingType;
    }

    return false;
}
} // end of namespace

namespace jsdialog
{

bool isIgnored(std::u16string_view rUIFile)
{
    return isInMap(IgnoredList, rUIFile, JSDialogEnabledType::Ignore);
}

bool isBuilderEnabled(std::u16string_view rUIFile, bool bMobile)
{
    // mobile only dialogs
    if (bMobile)
    {
        if (isInMap(MobileDialogList, rUIFile, JSDialogEnabledType::MobileDialog))
            return true;
    }

    if (isInMap(CuiDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(SfxDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(ScalcDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(SwriterDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(SimpressDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(SdrawDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(SchartDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    if (isInMap(OtherDialogList, rUIFile, JSDialogEnabledType::Dialog))
        return true;

    const char* pEnabledDialog = getenv("SAL_JSDIALOG_ENABLE");
    if (pEnabledDialog)
    {
        OUString sAllEnabledDialogs(pEnabledDialog, strlen(pEnabledDialog), RTL_TEXTENCODING_UTF8);
        std::vector<OUString> aEnabledDialogsVector
            = comphelper::string::split(sAllEnabledDialogs, ':');
        for (const auto& rDialog : aEnabledDialogsVector)
        {
            if (rUIFile == rDialog)
                return true;
        }
    }

    return false;
}

bool isBuilderEnabledForPopup(std::u16string_view rUIFile)
{
    return isInMap(PopupList, rUIFile, JSDialogEnabledType::Popup);
}

bool isBuilderEnabledForMenu(std::u16string_view rUIFile)
{
    return isInMap(MenuList, rUIFile, JSDialogEnabledType::Menu);
}

bool isBuilderEnabledForSidebar(std::u16string_view rUIFile)
{
    return isInMap(SidebarList, rUIFile, JSDialogEnabledType::Sidebar);
}

bool isInterimBuilderEnabledForNotebookbar(std::u16string_view rUIFile)
{
    return isInMap(NotebookbarList, rUIFile, JSDialogEnabledType::Notebookbar);
}

bool isBuilderEnabledForAddressInput(std::u16string_view rUIFile)
{
    return isInMap(AddressInputList, rUIFile, JSDialogEnabledType::AddressInput);
}

bool isBuilderEnabledForFormulabar(std::u16string_view rUIFile)
{
    return isInMap(FormulabarList, rUIFile, JSDialogEnabledType::Formulabar);
}
} // end of jsdialog

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
