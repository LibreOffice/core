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
#include <frozen/unordered_set.h>
#include <jsdialog/enabled.hxx>
#include <o3tl/string_view.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vector>

namespace
{
// ========== IGNORED ======================================================= //

constexpr auto IgnoredList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/simpress/ui/tabviewbar.ui" },
        { u"modules/swriter/ui/annotation.ui" },
        { u"sfx/ui/deck.ui" },
        { u"sfx/ui/extrabutton.ui" },
        { u"sfx/ui/infobar.ui" },
        { u"sfx/ui/tabbar.ui" },
        { u"sfx/ui/tabbarcontents.ui" },
        { u"svt/ui/scrollbars.ui" },
        { u"svt/ui/tabbuttons.ui" }, // Calc -> sheet tabs on the bottom
        { u"svt/ui/tabbuttonsmirrored.ui" }, // as above
        { u"svx/ui/measurewidthbar.ui" },
        { u"svx/ui/selectionmenu.ui" },
        { u"svx/ui/stylemenu.ui" },
        { u"svx/ui/toolbarpopover.ui" },
        { u"modules/scalc/ui/dropmenu.ui"}, // Calc -> Navigator -> right click
        { u"modules/sdraw/ui/navigatorcontextmenu.ui" }, // Impress -> Navigator -> right click
        { u"modules/swriter/ui/navigatorcontextmenu.ui" }, // Writer -> Navigator -> right click
        { u"sfx/ui/quickfind.ui" },
    });

// ========== MOBILE DIALOGS ================================================= //

constexpr auto MobileDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/swriter/ui/watermarkdialog.ui" },
        { u"modules/swriter/ui/wordcount-mobile.ui" },
        { u"svx/ui/findreplacedialog-mobile.ui" }
    });

// ========== DIALOGS ======================================================= //
// Split into few - to allow constexpr and manage order

constexpr auto CuiDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // cui
        { u"cui/ui/areatabpage.ui" },
        { u"cui/ui/areadialog.ui" },
        { u"cui/ui/asiantypography.ui" },
        { u"cui/ui/borderpage.ui" },
        { u"cui/ui/bulletandposition.ui" },
        { u"cui/ui/cellalignment.ui" },
        { u"cui/ui/charnamepage.ui" },
        { u"cui/ui/colorpage.ui" },
        { u"cui/ui/colorpickerdialog.ui" },
        { u"cui/ui/croppage.ui" },
        { u"cui/ui/diagramdialog.ui" },
        { u"cui/ui/effectspage.ui" },
        { u"cui/ui/eventassigndialog.ui" },
        { u"cui/ui/fontfeaturesdialog.ui" },
        { u"cui/ui/formatcellsdialog.ui" },
        { u"cui/ui/formatnumberdialog.ui" },
        { u"cui/ui/gradientpage.ui" },
        { u"cui/ui/hatchpage.ui" },
        { u"cui/ui/hyperlinkdialog.ui" },
        { u"cui/ui/hyperlinkinternetpage.ui" },
        { u"cui/ui/hyperlinkmailpage.ui" },
        { u"cui/ui/hyperlinkmarkdialog.ui" },
        { u"cui/ui/hyperlinkdocpage.ui" },
        { u"cui/ui/imagetabpage.ui" },
        { u"cui/ui/linedialog.ui" },
        { u"cui/ui/lineendstabpage.ui" },
        { u"cui/ui/linestyletabpage.ui" },
        { u"cui/ui/linetabpage.ui" },
        { u"cui/ui/macroselectordialog.ui" },
        { u"cui/ui/namedialog.ui" },
        { u"cui/ui/numberingformatpage.ui" },
        { u"cui/ui/numberingoptionspage.ui" },
        { u"cui/ui/numberingpositionpage.ui" },
        { u"cui/ui/objectnamedialog.ui" },
        { u"cui/ui/objecttitledescdialog.ui" },
        { u"cui/ui/optlingupage.ui" },
        { u"cui/ui/pageformatpage.ui" },
        { u"cui/ui/paragalignpage.ui" },
        { u"cui/ui/paraindentspacing.ui" },
        { u"cui/ui/paratabspage.ui" },
        { u"cui/ui/password.ui" },
        { u"cui/ui/pastespecial.ui" },
        { u"cui/ui/patterntabpage.ui" },
        { u"cui/ui/pickbulletpage.ui" },
        { u"cui/ui/pickgraphicpage.ui" },
        { u"cui/ui/picknumberingpage.ui" },
        { u"cui/ui/pickoutlinepage.ui" },
        { u"cui/ui/positionpage.ui" },
        { u"cui/ui/positionsizedialog.ui" },
        { u"cui/ui/possizetabpage.ui" },
        { u"cui/ui/rotationtabpage.ui" },
        { u"cui/ui/shadowtabpage.ui" },
        { u"cui/ui/slantcornertabpage.ui" },
        { u"cui/ui/spinbox.ui" },
        { u"cui/ui/queryduplicatedialog.ui" },
        { u"cui/ui/similaritysearchdialog.ui" },
        { u"cui/ui/specialcharacters.ui" },
        { u"cui/ui/spellingdialog.ui" },
        { u"cui/ui/spelloptionsdialog.ui" },
        { u"cui/ui/splitcellsdialog.ui" },
        { u"cui/ui/textflowpage.ui" },
        { u"cui/ui/thesaurus.ui" },
        { u"cui/ui/transparencytabpage.ui" },
        { u"cui/ui/twolinespage.ui" },
        { u"cui/ui/widgettestdialog.ui" },
        { u"cui/ui/qrcodegen.ui" }
    });

constexpr auto SfxDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // sfx
        { u"sfx/ui/cmisinfopage.ui" },
        { u"sfx/ui/custominfopage.ui" },
        { u"sfx/ui/descriptioninfopage.ui" },
        { u"sfx/ui/documentinfopage.ui" },
        { u"sfx/ui/documentfontspage.ui" },
        { u"sfx/ui/documentpropertiesdialog.ui" },
        { u"sfx/ui/editdurationdialog.ui" },
        { u"svx/ui/headfootformatpage.ui" },
        { u"sfx/ui/linefragment.ui" },
        { u"sfx/ui/managestylepage.ui" },
        { u"sfx/ui/newstyle.ui" },
        { u"sfx/ui/password.ui" }
    });

constexpr auto ScalcDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // scalc
        { u"modules/scalc/ui/advancedfilterdialog.ui" },
        { u"modules/scalc/ui/analysisofvariancedialog.ui" },
        { u"modules/scalc/ui/cellprotectionpage.ui" },
        { u"modules/scalc/ui/chardialog.ui" },
        { u"modules/scalc/ui/chisquaretestdialog.ui" },
        { u"modules/scalc/ui/colwidthdialog.ui" },
        { u"modules/scalc/ui/conditionaleasydialog.ui" },
        { u"modules/scalc/ui/condformatmanager.ui" },
        { u"modules/scalc/ui/correlationdialog.ui" },
        { u"modules/scalc/ui/covariancedialog.ui" },
        { u"modules/scalc/ui/datafielddialog.ui" },
        { u"modules/scalc/ui/datafieldoptionsdialog.ui" },
        { u"modules/scalc/ui/definename.ui" },
        { u"modules/scalc/ui/deletecells.ui" },
        { u"modules/scalc/ui/deletecontents.ui" },
        { u"modules/scalc/ui/descriptivestatisticsdialog.ui" },
        { u"modules/scalc/ui/erroralerttabpage.ui" },
        { u"modules/scalc/ui/exponentialsmoothingdialog.ui" },
        { u"modules/scalc/ui/formatcellsdialog.ui" },
        { u"modules/scalc/ui/fourieranalysisdialog.ui" },
        { u"modules/scalc/ui/goalseekdlg.ui" },
        { u"modules/scalc/ui/gotosheetdialog.ui" },
        { u"modules/scalc/ui/groupdialog.ui" },
        { u"modules/scalc/ui/headerfootercontent.ui" },
        { u"modules/scalc/ui/headerfooterdialog.ui" },
        { u"modules/scalc/ui/imoptdialog.ui" },
        { u"modules/scalc/ui/insertcells.ui" },
        { u"modules/scalc/ui/managenamesdialog.ui" },
        { u"modules/scalc/ui/mergecellsdialog.ui" },
        { u"modules/scalc/ui/movecopysheet.ui" },
        { u"modules/scalc/ui/movingaveragedialog.ui" },
        { u"modules/scalc/ui/optimalcolwidthdialog.ui" },
        { u"modules/scalc/ui/optimalrowheightdialog.ui" },
        { u"modules/scalc/ui/pagetemplatedialog.ui" },
        { u"modules/scalc/ui/paratemplatedialog.ui" },
        { u"modules/scalc/ui/pastespecial.ui" },
        { u"modules/scalc/ui/pivotfielddialog.ui" },
        { u"modules/scalc/ui/pivottablelayoutdialog.ui" },
        { u"modules/scalc/ui/protectsheetdlg.ui" },
        { u"modules/scalc/ui/regressiondialog.ui" },
        { u"modules/scalc/ui/rowheightdialog.ui" },
        { u"modules/scalc/ui/samplingdialog.ui" },
        { u"modules/scalc/ui/selectsource.ui" },
        { u"modules/scalc/ui/sheetprintpage.ui" },
        { u"modules/scalc/ui/simplerefdialog.ui" },
        { u"modules/scalc/ui/sortcriteriapage.ui" },
        { u"modules/scalc/ui/sortdialog.ui" },
        { u"modules/scalc/ui/sortkey.ui" },
        { u"modules/scalc/ui/sortoptionspage.ui" },
        { u"modules/scalc/ui/sparklinedialog.ui" },
        { u"modules/scalc/ui/standardfilterdialog.ui" },
        { u"modules/scalc/ui/statisticsinfopage.ui" },
        { u"modules/scalc/ui/textimportcsv.ui" },
        { u"modules/scalc/ui/ttestdialog.ui" },
        { u"modules/scalc/ui/ungroupdialog.ui" },
        { u"modules/scalc/ui/validationcriteriapage.ui" },
        { u"modules/scalc/ui/validationdialog.ui" },
        { u"modules/scalc/ui/validationhelptabpage.ui" },
        { u"modules/scalc/ui/warnautocorrect.ui" },
        { u"modules/scalc/ui/ztestdialog.ui" },
        { u"modules/scalc/ui/definedatabaserangedialog.ui" },
        { u"modules/scalc/ui/selectrange.ui" },
        { u"modules/scalc/ui/selectsheetviewdialog.ui" },
        { u"modules/scalc/ui/definetablerangedialog.ui" },
    });

constexpr auto SwriterDialogList
= frozen::make_unordered_set<std::u16string_view>({
        // swriter
        { u"modules/swriter/ui/bulletsandnumbering.ui" },
        { u"modules/swriter/ui/captionoptions.ui" },
        { u"modules/swriter/ui/characterproperties.ui" },
        { u"modules/swriter/ui/columndialog.ui" },
        { u"modules/swriter/ui/columnpage.ui" },
        { u"modules/swriter/ui/contentcontroldlg.ui" },
        { u"modules/swriter/ui/contentcontrollistitemdlg.ui" },
        { u"modules/swriter/ui/dropcapspage.ui" },
        { u"modules/swriter/ui/dropdownfielddialog.ui" },
        { u"modules/swriter/ui/editsectiondialog.ui" },
        { u"modules/swriter/ui/endnotepage.ui" },
        { u"modules/swriter/ui/editfielddialog.ui" },
        { u"modules/swriter/ui/fielddialog.ui" },
        { u"modules/swriter/ui/flddocumentpage.ui" },
        { u"modules/swriter/ui/fldvarpage.ui" },
        { u"modules/swriter/ui/flddocinfopage.ui" },
        { u"modules/swriter/ui/fldrefpage.ui" },
        { u"modules/swriter/ui/fldfuncpage.ui"},
        { u"modules/swriter/ui/footendnotedialog.ui" },
        { u"modules/swriter/ui/footnoteareapage.ui" },
        { u"modules/swriter/ui/footnotepage.ui" },
        { u"modules/swriter/ui/footnotesendnotestabpage.ui" },
        { u"modules/swriter/ui/formatsectiondialog.ui" },
        { u"modules/swriter/ui/formattablepage.ui" },
        { u"modules/swriter/ui/framedialog.ui" },
        { u"modules/swriter/ui/frmaddpage.ui" },
        { u"modules/swriter/ui/frmurlpage.ui" },
        { u"modules/swriter/ui/frmtypepage.ui" },
        { u"modules/swriter/ui/indentpage.ui" },
        { u"modules/swriter/ui/indexentry.ui" },
        { u"modules/swriter/ui/inforeadonlydialog.ui" },
        { u"modules/swriter/ui/insertbreak.ui" },
        { u"modules/swriter/ui/insertcaption.ui" },
        { u"modules/swriter/ui/insertsectiondialog.ui" },
        { u"modules/swriter/ui/linenumbering.ui" },
        { u"modules/swriter/ui/newuserindexdialog.ui" },
        { u"modules/swriter/ui/numparapage.ui" },
        { u"modules/swriter/ui/objectdialog.ui" },
        { u"modules/swriter/ui/pagenumberdlg.ui" },
        { u"modules/swriter/ui/paradialog.ui" },
        { u"modules/swriter/ui/picturedialog.ui" },
        { u"modules/swriter/ui/picturepage.ui" },
        { u"modules/swriter/ui/renameobjectdialog.ui" },
        { u"modules/swriter/ui/sectionpage.ui" },
        { u"modules/swriter/ui/sortdialog.ui" },
        { u"modules/swriter/ui/splittable.ui" },
        { u"modules/swriter/ui/tablecolumnpage.ui" },
        { u"modules/swriter/ui/tableproperties.ui" },
        { u"modules/swriter/ui/tabletextflowpage.ui" },
        { u"modules/swriter/ui/templatedialog1.ui" },
        { u"modules/swriter/ui/templatedialog2.ui" },
        { u"modules/swriter/ui/templatedialog8.ui" },
        { u"modules/swriter/ui/textgridpage.ui" },
        { u"modules/swriter/ui/titlepage.ui" },
        { u"modules/swriter/ui/tocdialog.ui" },
        { u"modules/swriter/ui/tocentriespage.ui" },
        { u"modules/swriter/ui/tocindexpage.ui" },
        { u"modules/swriter/ui/tocstylespage.ui" },
        { u"modules/swriter/ui/translationdialog.ui" },
        { u"modules/swriter/ui/watermarkdialog.ui" },
        { u"modules/swriter/ui/wordcount.ui" },
        { u"modules/swriter/ui/wrappage.ui" }
});

constexpr auto SimpressDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // simpress
        { u"modules/simpress/ui/customanimationeffecttab.ui" },
        { u"modules/simpress/ui/customanimationproperties.ui" },
        { u"modules/simpress/ui/customanimationtexttab.ui" },
        { u"modules/simpress/ui/customanimationtimingtab.ui" },
        { u"modules/simpress/ui/headerfooterdialog.ui" },
        { u"modules/simpress/ui/headerfootertab.ui" },
        { u"modules/simpress/ui/interactiondialog.ui" },
        { u"modules/simpress/ui/interactionpage.ui" }
    });

constexpr auto SdrawDialogList
    = frozen::make_unordered_set<std::u16string_view>({
    // sdraw
    { u"modules/sdraw/ui/drawchardialog.ui" },
    { u"modules/sdraw/ui/drawpagedialog.ui" },
    { u"modules/sdraw/ui/drawparadialog.ui" }
    });

constexpr auto SchartDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // schart
        { u"modules/schart/ui/attributedialog.ui" },
        { u"modules/schart/ui/charttypedialog.ui" },
        { u"modules/schart/ui/datarangedialog.ui" },
        { u"modules/schart/ui/insertaxisdlg.ui" },
        { u"modules/schart/ui/inserttitledlg.ui" },
        { u"modules/schart/ui/smoothlinesdlg.ui" },
        { u"modules/schart/ui/steppedlinesdlg.ui" },
        { u"modules/schart/ui/tp_ChartColorPalette.ui" },
        { u"modules/schart/ui/tp_ChartType.ui" },
        { u"modules/schart/ui/tp_DataLabel.ui" },
        { u"modules/schart/ui/tp_DataSource.ui" },
        { u"modules/schart/ui/tp_RangeChooser.ui" },
        { u"modules/schart/ui/tp_Trendline.ui" },
        { u"modules/schart/ui/wizelementspage.ui" },
        { u"modules/schart/ui/tp_axisLabel.ui" },
        { u"modules/schart/ui/tp_AxisPositions.ui" },
        { u"modules/schart/ui/tp_Scale.ui" },
        { u"modules/schart/ui/tp_SeriesToAxis.ui" },

    });

constexpr auto SmathDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // smath
        { u"modules/smath/ui/alignmentdialog.ui" },
        { u"modules/smath/ui/fontsizedialog.ui" },
        { u"modules/smath/ui/fonttypedialog.ui" },
        { u"modules/smath/ui/spacingdialog.ui" },
    });

constexpr auto OtherDialogList
    = frozen::make_unordered_set<std::u16string_view>({
        // formula
        { u"formula/ui/formuladialog.ui" },
        { u"formula/ui/functionpage.ui" },
        { u"formula/ui/parameter.ui" },
        { u"formula/ui/structpage.ui" },
        // svx
        { u"svx/ui/acceptrejectchangesdialog.ui" },
        { u"svx/ui/compressgraphicdialog.ui" },
        { u"svx/ui/findreplacedialog.ui" },
        { u"svx/ui/fontworkgallerydialog.ui" },
        { u"svx/ui/gotopagedialog.ui" },
        { u"svx/ui/headfootformatpage.ui" },
        { u"svx/ui/redlinecontrol.ui" },
        { u"svx/ui/redlinefilterpage.ui" },
        { u"svx/ui/redlineviewpage.ui" },
        { u"svx/ui/themecoloreditdialog.ui" },
        { u"svx/ui/themedialog.ui" },
        // uui
        { u"uui/ui/logindialog.ui" },
        { u"uui/ui/macrowarnmedium.ui" },
        // vcl
        { u"vcl/ui/wizard.ui" },
        // filter
        { u"filter/ui/pdfgeneralpage.ui" },
        { u"filter/ui/pdflinkspage.ui" },
        { u"filter/ui/pdfoptionsdialog.ui" },
        { u"filter/ui/pdfsecuritypage.ui" },
        { u"filter/ui/pdfsignpage.ui" },
        { u"filter/ui/pdfuserinterfacepage.ui" },
        { u"filter/ui/pdfviewpage.ui" },
        { u"filter/ui/warnpdfdialog.ui" },
        // writerperfect
        { u"writerperfect/ui/exportepub.ui" },
        { u"writerperfect/ui/wpftencodingdialog.ui" },
        // xmlsec
        { u"xmlsec/ui/certgeneral.ui" },
        { u"xmlsec/ui/certpage.ui" },
        { u"xmlsec/ui/digitalsignaturesdialog.ui" },
        { u"xmlsec/ui/selectcertificatedialog.ui" },
        { u"xmlsec/ui/viewcertdialog.ui" }
    });

// ========== POPUP ====================================================== //

constexpr auto PopupList
    = frozen::make_unordered_set<std::u16string_view>({
        // scalc
        { u"modules/scalc/ui/filterdropdown.ui" },
        { u"modules/scalc/ui/filterlist.ui" },
        { u"modules/scalc/ui/filtersubdropdown.ui" },
        { u"modules/scalc/ui/floatingborderstyle.ui" },
        { u"modules/scalc/ui/floatinglinestyle.ui" },
        // schart
        { u"modules/schart/ui/chartcolorpalettepopup.ui" },
        // svt
        { u"svt/ui/datewindow.ui" },
        { u"svt/ui/linewindow.ui" },
        // svx
        { u"svx/ui/colorwindow.ui" },
        { u"svx/ui/currencywindow.ui" },
        { u"svx/ui/floatingareastyle.ui" },
        { u"svx/ui/floatingframeborder.ui" },
        { u"svx/ui/floatinglineend.ui" },
        { u"svx/ui/floatinglineproperty.ui" },
        { u"svx/ui/floatinglinestyle.ui" },
        { u"svx/ui/fontworkalignmentcontrol.ui" },
        { u"svx/ui/fontworkcharacterspacingcontrol.ui" },
        { u"svx/ui/numberingwindow.ui" },
        { u"svx/ui/paralinespacingcontrol.ui" },
        { u"svx/ui/textcharacterspacingcontrol.ui" },
        { u"svx/ui/textunderlinecontrol.ui" }
    });

// ========== MENU ======================================================= //

constexpr auto MenuList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"sfx/ui/stylecontextmenu.ui" },
        { u"modules/simpress/ui/mastermenu.ui" },
        { u"modules/simpress/ui/layoutmenu.ui" }
    });

// ========== SIDEBAR ==================================================== //

constexpr auto SidebarList
    = frozen::make_unordered_set<std::u16string_view>({
        // scalc
        { u"modules/scalc/ui/functionpanel.ui" },
        { u"modules/scalc/ui/sidebaralignment.ui" },
        { u"modules/scalc/ui/sidebarcellappearance.ui" },
        { u"modules/scalc/ui/sidebardatabase.ui" },
        { u"modules/scalc/ui/sidebarnumberformat.ui" },
        // schart
        { u"modules/schart/ui/sidebaraxis.ui" },
        { u"modules/schart/ui/sidebarcolors.ui" },
        { u"modules/schart/ui/sidebarelements.ui" },
        { u"modules/schart/ui/sidebarerrorbar.ui" },
        { u"modules/schart/ui/sidebarseries.ui" },
        { u"modules/schart/ui/sidebartype.ui" },
        // simpress
        { u"modules/simpress/ui/customanimationfragment.ui" },
        { u"modules/simpress/ui/customanimationspanel.ui" },
        { u"modules/simpress/ui/layoutpanel.ui" },
        { u"modules/simpress/ui/sidebarslidebackground.ui" },
        { u"modules/simpress/ui/tabledesignpanel.ui" },
        // smath
        { u"modules/smath/ui/sidebarelements_math.ui" },
        { u"modules/smath/ui/sidebarproperties_math.ui" },
        // swriter
        { u"modules/swriter/ui/a11ycheckissuespanel.ui" },
        { u"modules/swriter/ui/managechangessidebar.ui" },
        { u"modules/swriter/ui/pagefooterpanel.ui" },
        { u"modules/swriter/ui/pageformatpanel.ui" },
        { u"modules/swriter/ui/pageheaderpanel.ui" },
        { u"modules/swriter/ui/pagestylespanel.ui" },
        { u"modules/swriter/ui/sidebarstylepresets.ui" },
        { u"modules/swriter/ui/sidebartableedit.ui" },
        { u"modules/swriter/ui/sidebartheme.ui" },
        { u"modules/swriter/ui/sidebarwrap.ui" },
        // sfx
        { u"sfx/ui/panel.ui" },
        { u"sfx/ui/templatepanel.ui" },
        // svx
        { u"svx/ui/accessibilitycheckentry.ui" },
        { u"svx/ui/accessibilitychecklevel.ui" },
        { u"svx/ui/defaultshapespanel.ui" },
        { u"svx/ui/inspectortextpanel.ui" },
        { u"svx/ui/mediaplayback.ui" },
        { u"svx/ui/sidebararea.ui" },
        { u"svx/ui/sidebareffect.ui" },
        { u"svx/ui/sidebarempty.ui" },
        { u"svx/ui/sidebarfontwork.ui" },
        { u"svx/ui/sidebargallery.ui" },
        { u"svx/ui/sidebargraphic.ui" },
        { u"svx/ui/sidebarline.ui" },
        { u"svx/ui/sidebarlists.ui" },
        { u"svx/ui/sidebarparagraph.ui" },
        { u"svx/ui/sidebarpossize.ui" },
        { u"svx/ui/sidebarshadow.ui" },
        { u"svx/ui/sidebarstylespanel.ui" },
        { u"svx/ui/sidebartextcolumnspanel.ui" },
        { u"svx/ui/sidebartexteffect.ui" },
        { u"svx/ui/sidebartextpanel.ui" }
    });

// ========== NAVIGATOR ================================================= //
constexpr auto NavigatorList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/swriter/ui/navigatorpanel.ui"},
        { u"modules/scalc/ui/navigatorpanel.ui"},
        { u"modules/simpress/ui/navigatorpanel.ui"}
});

// ========== QUICKFIND ================================================= //
constexpr auto QuickFindList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/swriter/ui/sidebarquickfind.ui" },
});


// ========== NOTEBOOKBAR ================================================= //

constexpr auto NotebookbarList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/scalc/ui/numberbox.ui" },
        { u"modules/scalc/ui/sheetviewbox.ui" },
        { u"modules/scalc/ui/tablestylesbox.ui" },
        { u"svx/ui/fontnamebox.ui" },
        { u"svx/ui/fontsizebox.ui" },
        { u"svx/ui/stylespreview.ui" },
        { u"svx/ui/themeselectorpanel.ui" },
        // not interim builder, but regular builder:
        { u"modules/simpress/ui/masterpagepanelall.ui" },
        { u"modules/simpress/ui/slidetransitionspanel.ui" }
    });

// ========== ADDRESSINPUT ================================================ //

constexpr auto AddressInputList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/scalc/ui/posbox.ui" }
    });

// ========== FORMULABAR ================================================== //

constexpr auto FormulabarList
    = frozen::make_unordered_set<std::u16string_view>({
        { u"modules/scalc/ui/inputbar.ui" }
    });

// ========== LOOKUP ====================================================== //

inline bool isInMap(const auto& rList, std::u16string_view rUIFile)
{
    auto aFound = rList.find(rUIFile);
    if (aFound != rList.end())
        return true;
    return false;
}

inline bool isEnabledAtRunTime(std::u16string_view rUIFile)
{
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
} // end of namespace

namespace jsdialog
{

bool isIgnored(std::u16string_view rUIFile)
{
    return isInMap(IgnoredList, rUIFile);
}

bool isBuilderEnabled(std::u16string_view rUIFile, bool bMobile)
{
    // mobile only dialogs
    if (bMobile)
    {
        if (isInMap(MobileDialogList, rUIFile))
            return true;
    }

    if (isInMap(CuiDialogList, rUIFile))
        return true;

    if (isInMap(SfxDialogList, rUIFile))
        return true;

    if (isInMap(ScalcDialogList, rUIFile))
        return true;

    if (isInMap(SwriterDialogList, rUIFile))
        return true;

    if (isInMap(SimpressDialogList, rUIFile))
        return true;

    if (isInMap(SdrawDialogList, rUIFile))
        return true;

    if (isInMap(SchartDialogList, rUIFile))
        return true;

    if (isInMap(SmathDialogList, rUIFile))
        return true;

    if (isInMap(OtherDialogList, rUIFile))
        return true;

    return isEnabledAtRunTime(rUIFile);
}

bool isBuilderEnabledForPopup(std::u16string_view rUIFile)
{
    return isInMap(PopupList, rUIFile);
}

bool isBuilderEnabledForMenu(std::u16string_view rUIFile)
{
    return isInMap(MenuList, rUIFile);
}

bool isBuilderEnabledForSidebar(std::u16string_view rUIFile)
{
    return isInMap(SidebarList, rUIFile);

}

bool isBuilderEnabledForNavigator(std::u16string_view rUIFile)
{
        return isInMap(NavigatorList, rUIFile);
}

bool isBuilderEnabledForQuickFind(std::u16string_view rUIFile)
{
    return isInMap(QuickFindList, rUIFile);
}

bool isInterimBuilderEnabledForNotebookbar(std::u16string_view rUIFile)
{
    return isInMap(NotebookbarList, rUIFile);
}

bool isBuilderEnabledForAddressInput(std::u16string_view rUIFile)
{
    return isInMap(AddressInputList, rUIFile);
}

bool isBuilderEnabledForFormulabar(std::u16string_view rUIFile)
{
    return isInMap(FormulabarList, rUIFile);
}

std::vector<OUString> completeWriterDialogList(const o3tl::sorted_vector<OUString>& entries)
{
    std::vector<OUString> missing;
    for (const auto& entry : SwriterDialogList)
    {
        OUString sEntry(entry);
        if (!entries.contains(sEntry))
            missing.push_back(sEntry);
    }
    return missing;
}

std::vector<OUString> completeWriterSidebarList(const o3tl::sorted_vector<OUString>& entries)
{
    std::vector<OUString> missing;
    for (const auto& entry : SidebarList)
    {
        OUString sEntry(entry);
        // Skip these ones, I don't think they can appear in practice
        if (entry == u"modules/swriter/ui/managechangessidebar.ui" ||
            entry == u"modules/swriter/ui/pagefooterpanel.ui" ||
            entry == u"modules/swriter/ui/pageheaderpanel.ui" ||
            entry == u"modules/swriter/ui/pagestylespanel.ui" ||
            entry == u"modules/swriter/ui/sidebarstylepresets.ui" ||
            entry == u"modules/swriter/ui/sidebartheme.ui")
            continue;
        else if (sEntry.startsWith("modules/swriter/") && !entries.contains(sEntry))
            missing.push_back(sEntry);
    }
    return missing;
}

std::vector<OUString> completeCalcDialogList(const o3tl::sorted_vector<OUString>& entries)
{
    std::vector<OUString> missing;
    for (const auto& entry : ScalcDialogList)
    {
        OUString sEntry(entry);
        if (!entries.contains(sEntry))
            missing.push_back(sEntry);
    }
    return missing;
}

std::vector<OUString> completeCalcSidebarList(const o3tl::sorted_vector<OUString>& entries)
{
    std::vector<OUString> missing;
    for (const auto& entry : SidebarList)
    {
        OUString sEntry(entry);
        if (sEntry.startsWith("modules/scalc/") && !entries.contains(sEntry))
            missing.push_back(sEntry);
    }
    return missing;
}

std::vector<OUString> completeCommonSidebarList(const o3tl::sorted_vector<OUString>& entries)
{
    std::vector<OUString> missing;
    for (const auto& entry : SidebarList)
    {
        OUString sEntry(entry);

        // consider schart and smath as 'common', but other modules specific to a toplevel application
        if (sEntry.startsWith("modules/") && !sEntry.startsWith("modules/schart/") && !sEntry.startsWith("modules/smath/"))
            continue;
        //TODO:  This one should be selectable, but that seems to be broken
        else if (entry == u"modules/schart/ui/sidebarerrorbar.ui")
            continue;
        // Skip this one, it can't appear in practice
        else if (entry == u"modules/smath/ui/sidebarproperties_math.ui")
            continue;
        // Skip this one, theme related, disabled at the moment
        else if (entry == u"modules/schart/ui/sidebarcolors.ui")
            continue;
        // Skip this one, theme related, disabled at the moment
        else if (entry == u"modules/schart/ui/sidebargradients.ui")
            continue;
        // Skip this one, theme related, disabled at the moment
        else if (entry == u"modules/schart/ui/sidebartheme.ui")
            continue;
        // Skip this one, in practice it appears in draw/impress
        // TODO: it should probably be made to appear in writer too
        else if (entry == u"svx/ui/mediaplayback.ui")
            continue;
        // Skip this one, I don't think it can appear in practice
        else if (entry == u"svx/ui/sidebargallery.ui")
            continue;
        // Skip this one, its context means it cannot appear in writer
        else if (entry == u"svx/ui/sidebarshadow.ui")
            continue;
        // Skip this one, its context means it cannot appear in writer
        else if (entry == u"svx/ui/sidebartexteffect.ui")
            continue;
        // Skip this one, its context means it cannot appear in writer
        else if (entry == u"svx/ui/sidebarlists.ui")
            continue;
        // Skip this one, its context means it can only appear in draw/impress
        else if (entry == u"svx/ui/defaultshapespanel.ui")
            continue;
        else if (!entries.contains(sEntry))
            missing.push_back(sEntry);
    }
    return missing;
}

std::vector<OUString> completeCommonDialogList(const o3tl::sorted_vector<OUString>& entries,
                                               /*LibreOfficeKitDocumentType*/ int docType,
                                               bool linguisticDataAvailable)
{
    std::vector<OUString> missing;
    auto processCategory = [&](const auto& category) {
        for (const auto& entry : category)
        {
            if (!linguisticDataAvailable && (
                entry == u"cui/ui/thesaurus.ui" ||
                entry == u"cui/ui/spellingdialog.ui" ||
                entry == u"cui/ui/spelloptionsdialog.ui"))
            {
                // Skip the dialogs that can't be reached in the absense of
                // linguistic data.
                continue;
            }

            if (docType != LOK_DOCTYPE_TEXT)
            {
                // The 'writerperfect' ones are writer only
                if (o3tl::starts_with(entry, u"writerperfect"))
                    continue;
                // The manage changes dialog is only enabled in writer
                else if (entry == u"svx/ui/acceptrejectchangesdialog.ui")
                    continue;
            }

            if (docType == LOK_DOCTYPE_SPREADSHEET)
            {
                // Not supported in Calc
                if (entry == u"svx/ui/gotopagedialog.ui" ||
                    entry == u"cui/ui/splitcellsdialog.ui")
                {
                    continue;
                }
            }

            // Skip this one, I don't think it can appear in practice
            if (entry == u"sfx/ui/cmisinfopage.ui")
                continue;
            // Skip this one, I think it can only happen on loading
            // an archaic wordperfect file
            else if (entry == u"writerperfect/ui/wpftencodingdialog.ui")
                continue;
            // Skip this one, I don't think it can appear in practice
            else if (entry == u"cui/ui/colorpickerdialog.ui")
                continue;
            // Skip this one, is the query dialog about enabling overwrite
            // mode which is disabled in the default config
            else if (entry == u"cui/ui/querysetinsmodedialog.ui")
                continue;
            // Skip this one, its actually a sd-only one (with code in sd), but
            // somehow the .ui is in cui
            else if (entry == u"cui/ui/bulletandposition.ui")
                continue;
            // Skip this one for now, it requires smartart to already exist in a .docx
            else if (entry == u"cui/ui/diagramdialog.ui")
                continue;
            // Skip this one, it cannot appear in writer
            else if (entry == u"cui/ui/possizetabpage.ui")
                continue;
            // Skip this one, it cannot appear in writer, only calc in practice
            else if (entry == u"cui/ui/cellalignment.ui")
                continue;
            // Skip this one, it cannot appear in writer, only impress/draw in
            // practice
            else if (entry == u"cui/ui/formatcellsdialog.ui")
                continue;
            // Skip this one, it cannot appear in writer (until autotext dialogs
            // are enabled, in which case that is likely disabled. Maybe possible
            // from calc.
            else if (entry == u"cui/ui/eventassigndialog.ui")
                continue;
            // Skip this one, it is disabled in filter/source/pdf/impdialog.cxx
            // for kit mode
            else if (entry == u"filter/ui/pdfsignpage.ui")
                continue;
            // This, for the chart wizard, it is really only available in calc
            else if (entry == u"vcl/ui/wizard.ui")
                continue;
            // The warn dialog appears to be somewhat broken, at least the
            // existing tests for it are currently disabled and it doesn't
            // work for me
            else if (entry == u"uui/ui/macrowarnmedium.ui")
                continue;
            // This, for the formula dialog, are really only available in calc
            else if (entry == u"formula/ui/formuladialog.ui" ||
                     entry == u"formula/ui/functionpage.ui" ||
                     entry == u"formula/ui/parameter.ui" ||
                     entry == u"formula/ui/structpage.ui")
            {
                continue;
            }
            // Testing this requires hosting an image, or similar, with
            // a username+password
            else if (entry == u"uui/ui/logindialog.ui")
                continue;
            OUString sEntry(entry);
            if (!entries.contains(sEntry))
                missing.push_back(sEntry);
        }
    };
    processCategory(CuiDialogList);
    processCategory(SfxDialogList);
    processCategory(OtherDialogList);
    return missing;
}


} // end of jsdialog

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
