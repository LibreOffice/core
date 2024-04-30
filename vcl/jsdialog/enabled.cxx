/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/string.hxx>
#include <jsdialog/enabled.hxx>
#include <vector>

namespace jsdialog
{
bool isBuilderEnabled(std::u16string_view rUIFile, bool bMobile)
{
    // mobile only dialogs
    if (bMobile)
    {
        if (// swriter
            rUIFile == u"modules/swriter/ui/watermarkdialog.ui"
            || rUIFile == u"modules/swriter/ui/wordcount-mobile.ui"
            // svx
            || rUIFile == u"svx/ui/findreplacedialog-mobile.ui")
        {
            return true;
        }
    }

    if (// cui
        rUIFile == u"cui/ui/areatabpage.ui"
        || rUIFile == u"cui/ui/areadialog.ui"
        || rUIFile == u"cui/ui/asiantypography.ui"
        || rUIFile == u"cui/ui/borderpage.ui"
        || rUIFile == u"cui/ui/bulletandposition.ui"
        || rUIFile == u"cui/ui/cellalignment.ui"
        || rUIFile == u"cui/ui/charnamepage.ui"
        || rUIFile == u"cui/ui/colorpage.ui"
        || rUIFile == u"cui/ui/colorpickerdialog.ui"
        || rUIFile == u"cui/ui/croppage.ui"
        || rUIFile == u"cui/ui/effectspage.ui"
        || rUIFile == u"cui/ui/eventassigndialog.ui"
        || rUIFile == u"cui/ui/fontfeaturesdialog.ui"
        || rUIFile == u"cui/ui/formatcellsdialog.ui"
        || rUIFile == u"cui/ui/formatnumberdialog.ui"
        || rUIFile == u"cui/ui/gradientpage.ui"
        || rUIFile == u"cui/ui/hatchpage.ui"
        || rUIFile == u"cui/ui/hyperlinkdialog.ui"
        || rUIFile == u"cui/ui/hyperlinkinternetpage.ui"
        || rUIFile == u"cui/ui/hyperlinkmailpage.ui"
        || rUIFile == u"cui/ui/imagetabpage.ui"
        || rUIFile == u"cui/ui/linedialog.ui"
        || rUIFile == u"cui/ui/lineendstabpage.ui"
        || rUIFile == u"cui/ui/linestyletabpage.ui"
        || rUIFile == u"cui/ui/linetabpage.ui"
        || rUIFile == u"cui/ui/macroselectordialog.ui"
        || rUIFile == u"cui/ui/numberingformatpage.ui"
        || rUIFile == u"cui/ui/numberingoptionspage.ui"
        || rUIFile == u"cui/ui/numberingpositionpage.ui"
        || rUIFile == u"cui/ui/optlingupage.ui"
        || rUIFile == u"cui/ui/pageformatpage.ui"
        || rUIFile == u"cui/ui/paragalignpage.ui"
        || rUIFile == u"cui/ui/paraindentspacing.ui"
        || rUIFile == u"cui/ui/paratabspage.ui"
        || rUIFile == u"cui/ui/password.ui"
        || rUIFile == u"cui/ui/pastespecial.ui"
        || rUIFile == u"cui/ui/patterntabpage.ui"
        || rUIFile == u"cui/ui/pickbulletpage.ui"
        || rUIFile == u"cui/ui/pickgraphicpage.ui"
        || rUIFile == u"cui/ui/picknumberingpage.ui"
        || rUIFile == u"cui/ui/pickoutlinepage.ui"
        || rUIFile == u"cui/ui/positionpage.ui"
        || rUIFile == u"cui/ui/positionsizedialog.ui"
        || rUIFile == u"cui/ui/possizetabpage.ui"
        || rUIFile == u"cui/ui/rotationtabpage.ui"
        || rUIFile == u"cui/ui/shadowtabpage.ui"
        || rUIFile == u"cui/ui/slantcornertabpage.ui"
        || rUIFile == u"cui/ui/spinbox.ui"
        || rUIFile == u"cui/ui/queryduplicatedialog.ui"
        || rUIFile == u"cui/ui/similaritysearchdialog.ui"
        || rUIFile == u"cui/ui/specialcharacters.ui"
        || rUIFile == u"cui/ui/spellingdialog.ui"
        || rUIFile == u"cui/ui/spelloptionsdialog.ui"
        || rUIFile == u"cui/ui/splitcellsdialog.ui"
        || rUIFile == u"cui/ui/textflowpage.ui"
        || rUIFile == u"cui/ui/thesaurus.ui"
        || rUIFile == u"cui/ui/transparencytabpage.ui"
        || rUIFile == u"cui/ui/twolinespage.ui"
        || rUIFile == u"cui/ui/widgettestdialog.ui"
        || rUIFile == u"cui/ui/qrcodegen.ui"
        // formula
        || rUIFile == u"formula/ui/formuladialog.ui"
        || rUIFile == u"formula/ui/functionpage.ui"
        || rUIFile == u"formula/ui/parameter.ui"
        || rUIFile == u"formula/ui/structpage.ui"
        // scalc
        || rUIFile == u"modules/scalc/ui/advancedfilterdialog.ui"
        || rUIFile == u"modules/scalc/ui/analysisofvariancedialog.ui"
        || rUIFile == u"modules/scalc/ui/cellprotectionpage.ui"
        || rUIFile == u"modules/scalc/ui/chardialog.ui"
        || rUIFile == u"modules/scalc/ui/chisquaretestdialog.ui"
        || rUIFile == u"modules/scalc/ui/colwidthdialog.ui"
        || rUIFile == u"modules/scalc/ui/conditionaleasydialog.ui"
        || rUIFile == u"modules/scalc/ui/condformatmanager.ui"
        || rUIFile == u"modules/scalc/ui/correlationdialog.ui"
        || rUIFile == u"modules/scalc/ui/covariancedialog.ui"
        || rUIFile == u"modules/scalc/ui/datafielddialog.ui"
        || rUIFile == u"modules/scalc/ui/datafieldoptionsdialog.ui"
        || rUIFile == u"modules/scalc/ui/definename.ui"
        || rUIFile == u"modules/scalc/ui/deletecells.ui"
        || rUIFile == u"modules/scalc/ui/deletecontents.ui"
        || rUIFile == u"modules/scalc/ui/descriptivestatisticsdialog.ui"
        || rUIFile == u"modules/scalc/ui/erroralerttabpage.ui"
        || rUIFile == u"modules/scalc/ui/exponentialsmoothingdialog.ui"
        || rUIFile == u"modules/scalc/ui/formatcellsdialog.ui"
        || rUIFile == u"modules/scalc/ui/fourieranalysisdialog.ui"
        || rUIFile == u"modules/scalc/ui/goalseekdlg.ui"
        || rUIFile == u"modules/scalc/ui/groupdialog.ui"
        || rUIFile == u"modules/scalc/ui/headerfootercontent.ui"
        || rUIFile == u"modules/scalc/ui/headerfooterdialog.ui"
        || rUIFile == u"modules/scalc/ui/insertcells.ui"
        || rUIFile == u"modules/scalc/ui/managenamesdialog.ui"
        || rUIFile == u"modules/scalc/ui/movingaveragedialog.ui"
        || rUIFile == u"modules/scalc/ui/optimalcolwidthdialog.ui"
        || rUIFile == u"modules/scalc/ui/optimalrowheightdialog.ui"
        || rUIFile == u"modules/scalc/ui/pagetemplatedialog.ui"
        || rUIFile == u"modules/scalc/ui/paratemplatedialog.ui"
        || rUIFile == u"modules/scalc/ui/pastespecial.ui"
        || rUIFile == u"modules/scalc/ui/pivotfielddialog.ui"
        || rUIFile == u"modules/scalc/ui/pivottablelayoutdialog.ui"
        || rUIFile == u"modules/scalc/ui/protectsheetdlg.ui"
        || rUIFile == u"modules/scalc/ui/regressiondialog.ui"
        || rUIFile == u"modules/scalc/ui/rowheightdialog.ui"
        || rUIFile == u"modules/scalc/ui/samplingdialog.ui"
        || rUIFile == u"modules/scalc/ui/selectsource.ui"
        || rUIFile == u"modules/scalc/ui/sheetprintpage.ui"
        || rUIFile == u"modules/scalc/ui/simplerefdialog.ui"
        || rUIFile == u"modules/scalc/ui/sortcriteriapage.ui"
        || rUIFile == u"modules/scalc/ui/sortdialog.ui"
        || rUIFile == u"modules/scalc/ui/sortkey.ui"
        || rUIFile == u"modules/scalc/ui/sortoptionspage.ui"
        || rUIFile == u"modules/scalc/ui/sparklinedialog.ui"
        || rUIFile == u"modules/scalc/ui/standardfilterdialog.ui"
        || rUIFile == u"modules/scalc/ui/textimportcsv.ui"
        || rUIFile == u"modules/scalc/ui/ttestdialog.ui"
        || rUIFile == u"modules/scalc/ui/ungroupdialog.ui"
        || rUIFile == u"modules/scalc/ui/validationcriteriapage.ui"
        || rUIFile == u"modules/scalc/ui/validationdialog.ui"
        || rUIFile == u"modules/scalc/ui/validationhelptabpage.ui"
        || rUIFile == u"modules/scalc/ui/warnautocorrect.ui"
        || rUIFile == u"modules/scalc/ui/ztestdialog.ui"
        // schart
        || rUIFile == u"modules/schart/ui/attributedialog.ui"
        || rUIFile == u"modules/schart/ui/charttypedialog.ui"
        || rUIFile == u"modules/schart/ui/datarangedialog.ui"
        || rUIFile == u"modules/schart/ui/insertaxisdlg.ui"
        || rUIFile == u"modules/schart/ui/inserttitledlg.ui"
        || rUIFile == u"modules/schart/ui/smoothlinesdlg.ui"
        || rUIFile == u"modules/schart/ui/steppedlinesdlg.ui"
        || rUIFile == u"modules/schart/ui/tp_ChartType.ui"
        || rUIFile == u"modules/schart/ui/tp_DataSource.ui"
        || rUIFile == u"modules/schart/ui/tp_RangeChooser.ui"
        || rUIFile == u"modules/schart/ui/tp_Trendline.ui"
        || rUIFile == u"modules/schart/ui/wizelementspage.ui"
        // sdraw
        || rUIFile == u"modules/sdraw/ui/drawchardialog.ui"
        || rUIFile == u"modules/sdraw/ui/drawpagedialog.ui"
        || rUIFile == u"modules/sdraw/ui/drawparadialog.ui"
        // simpress
        || rUIFile == u"modules/simpress/ui/headerfooterdialog.ui"
        || rUIFile == u"modules/simpress/ui/headerfootertab.ui"
        // swriter
        || rUIFile == u"modules/swriter/ui/bulletsandnumbering.ui"
        || rUIFile == u"modules/swriter/ui/captionoptions.ui"
        || rUIFile == u"modules/swriter/ui/characterproperties.ui"
        || rUIFile == u"modules/swriter/ui/charurlpage.ui"
        || rUIFile == u"modules/swriter/ui/columndialog.ui"
        || rUIFile == u"modules/swriter/ui/columnpage.ui"
        || rUIFile == u"modules/swriter/ui/contentcontroldlg.ui"
        || rUIFile == u"modules/swriter/ui/contentcontrollistitemdlg.ui"
        || rUIFile == u"modules/swriter/ui/dropcapspage.ui"
        || rUIFile == u"modules/swriter/ui/dropdownfielddialog.ui"
        || rUIFile == u"modules/swriter/ui/editsectiondialog.ui"
        || rUIFile == u"modules/swriter/ui/endnotepage.ui"
        || rUIFile == u"modules/swriter/ui/footendnotedialog.ui"
        || rUIFile == u"modules/swriter/ui/footnoteareapage.ui"
        || rUIFile == u"modules/swriter/ui/footnotepage.ui"
        || rUIFile == u"modules/swriter/ui/footnotesendnotestabpage.ui"
        || rUIFile == u"modules/swriter/ui/formatsectiondialog.ui"
        || rUIFile == u"modules/swriter/ui/formattablepage.ui"
        || rUIFile == u"modules/swriter/ui/framedialog.ui"
        || rUIFile == u"modules/swriter/ui/frmaddpage.ui"
        || rUIFile == u"modules/swriter/ui/frmurlpage.ui"
        || rUIFile == u"modules/swriter/ui/frmtypepage.ui"
        || rUIFile == u"modules/swriter/ui/indentpage.ui"
        || rUIFile == u"modules/swriter/ui/indexentry.ui"
        || rUIFile == u"modules/swriter/ui/inforeadonlydialog.ui"
        || rUIFile == u"modules/swriter/ui/insertbreak.ui"
        || rUIFile == u"modules/swriter/ui/insertcaption.ui"
        || rUIFile == u"modules/swriter/ui/insertsectiondialog.ui"
        || rUIFile == u"modules/swriter/ui/linenumbering.ui"
        || rUIFile == u"modules/swriter/ui/newuserindexdialog.ui"
        || rUIFile == u"modules/swriter/ui/numparapage.ui"
        || rUIFile == u"modules/swriter/ui/objectdialog.ui"
        || rUIFile == u"modules/swriter/ui/pagenumberdlg.ui"
        || rUIFile == u"modules/swriter/ui/paradialog.ui"
        || rUIFile == u"modules/swriter/ui/picturedialog.ui"
        || rUIFile == u"modules/swriter/ui/picturepage.ui"
        || rUIFile == u"modules/swriter/ui/sectionpage.ui"
        || rUIFile == u"modules/swriter/ui/sortdialog.ui"
        || rUIFile == u"modules/swriter/ui/splittable.ui"
        || rUIFile == u"modules/swriter/ui/tablecolumnpage.ui"
        || rUIFile == u"modules/swriter/ui/tableproperties.ui"
        || rUIFile == u"modules/swriter/ui/tabletextflowpage.ui"
        || rUIFile == u"modules/swriter/ui/templatedialog1.ui"
        || rUIFile == u"modules/swriter/ui/templatedialog2.ui"
        || rUIFile == u"modules/swriter/ui/templatedialog8.ui"
        || rUIFile == u"modules/swriter/ui/textgridpage.ui"
        || rUIFile == u"modules/swriter/ui/titlepage.ui"
        || rUIFile == u"modules/swriter/ui/tocdialog.ui"
        || rUIFile == u"modules/swriter/ui/tocentriespage.ui"
        || rUIFile == u"modules/swriter/ui/tocindexpage.ui"
        || rUIFile == u"modules/swriter/ui/tocstylespage.ui"
        || rUIFile == u"modules/swriter/ui/translationdialog.ui"
        || rUIFile == u"modules/swriter/ui/watermarkdialog.ui"
        || rUIFile == u"modules/swriter/ui/wordcount.ui"
        || rUIFile == u"modules/swriter/ui/wrappage.ui"
        // sfx
        || rUIFile == u"sfx/ui/cmisinfopage.ui"
        || rUIFile == u"sfx/ui/custominfopage.ui"
        || rUIFile == u"sfx/ui/descriptioninfopage.ui"
        || rUIFile == u"sfx/ui/documentinfopage.ui"
        || rUIFile == u"sfx/ui/documentfontspage.ui"
        || rUIFile == u"sfx/ui/documentpropertiesdialog.ui"
        || rUIFile == u"sfx/ui/editdurationdialog.ui"
        || rUIFile == u"svx/ui/headfootformatpage.ui"
        || rUIFile == u"sfx/ui/linefragment.ui"
        || rUIFile == u"sfx/ui/managestylepage.ui"
        || rUIFile == u"sfx/ui/password.ui"
        // svx
        || rUIFile == u"svx/ui/acceptrejectchangesdialog.ui"
        || rUIFile == u"svx/ui/accessibilitycheckdialog.ui"
        || rUIFile == u"svx/ui/accessibilitycheckentry.ui"
        || rUIFile == u"svx/ui/compressgraphicdialog.ui"
        || rUIFile == u"svx/ui/findreplacedialog.ui"
        || rUIFile == u"svx/ui/fontworkgallerydialog.ui"
        || rUIFile == u"svx/ui/headfootformatpage.ui"
        || rUIFile == u"svx/ui/redlinecontrol.ui"
        || rUIFile == u"svx/ui/redlinefilterpage.ui"
        || rUIFile == u"svx/ui/redlineviewpage.ui"
        || rUIFile == u"svx/ui/themecoloreditdialog.ui"
        || rUIFile == u"svx/ui/themedialog.ui"
        // uui
        || rUIFile == u"uui/ui/macrowarnmedium.ui"
        // vcl
        || rUIFile == u"vcl/ui/wizard.ui"
        // filter
        || rUIFile == u"filter/ui/pdfgeneralpage.ui"
        || rUIFile == u"filter/ui/pdflinkspage.ui"
        || rUIFile == u"filter/ui/pdfoptionsdialog.ui"
        || rUIFile == u"filter/ui/pdfsecuritypage.ui"
        || rUIFile == u"filter/ui/pdfsignpage.ui"
        || rUIFile == u"filter/ui/pdfuserinterfacepage.ui"
        || rUIFile == u"filter/ui/pdfviewpage.ui"
        || rUIFile == u"filter/ui/warnpdfdialog.ui"
        // writerperfect
        || rUIFile == u"writerperfect/ui/exportepub.ui"
        // xmlsec
        || rUIFile == u"xmlsec/ui/certgeneral.ui"
        || rUIFile == u"xmlsec/ui/certpage.ui"
        || rUIFile == u"xmlsec/ui/digitalsignaturesdialog.ui"
        || rUIFile == u"xmlsec/ui/viewcertdialog.ui"
        )
    {
        return true;
    }

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
    if (// scalc
        rUIFile == u"modules/scalc/ui/filterdropdown.ui"
        || rUIFile == u"modules/scalc/ui/filterlist.ui"
        || rUIFile == u"modules/scalc/ui/filtersubdropdown.ui"
        || rUIFile == u"modules/scalc/ui/floatingborderstyle.ui"
        || rUIFile == u"modules/scalc/ui/floatinglinestyle.ui"
        // svt
        || rUIFile == u"svt/ui/datewindow.ui"
        || rUIFile == u"svt/ui/linewindow.ui"
        // svx
        || rUIFile == u"svx/ui/colorwindow.ui"
        || rUIFile == u"svx/ui/currencywindow.ui"
        || rUIFile == u"svx/ui/floatingareastyle.ui"
        || rUIFile == u"svx/ui/floatinglineend.ui"
        || rUIFile == u"svx/ui/floatinglineproperty.ui"
        || rUIFile == u"svx/ui/floatinglinestyle.ui"
        || rUIFile == u"svx/ui/fontworkalignmentcontrol.ui"
        || rUIFile == u"svx/ui/fontworkcharacterspacingcontrol.ui"
        || rUIFile == u"svx/ui/numberingwindow.ui"
        || rUIFile == u"svx/ui/paralinespacingcontrol.ui"
        || rUIFile == u"svx/ui/textcharacterspacingcontrol.ui"
        || rUIFile == u"svx/ui/textunderlinecontrol.ui")
        return true;

    return false;
}

bool isBuilderEnabledForSidebar(std::u16string_view rUIFile)
{
    return // scalc
        rUIFile == u"modules/scalc/ui/functionpanel.ui"
        || rUIFile == u"modules/scalc/ui/navigatorpanel.ui"
        || rUIFile == u"modules/scalc/ui/sidebaralignment.ui"
        || rUIFile == u"modules/scalc/ui/sidebarcellappearance.ui"
        || rUIFile == u"modules/scalc/ui/sidebarnumberformat.ui"
        // schart
        || rUIFile == u"modules/schart/ui/sidebaraxis.ui"
        || rUIFile == u"modules/schart/ui/sidebarelements.ui"
        || rUIFile == u"modules/schart/ui/sidebarerrorbar.ui"
        || rUIFile == u"modules/schart/ui/sidebarseries.ui"
        || rUIFile == u"modules/schart/ui/sidebartype.ui"
        // simpress
        || rUIFile == u"modules/simpress/ui/customanimationfragment.ui"
        || rUIFile == u"modules/simpress/ui/customanimationspanel.ui"
        || rUIFile == u"modules/simpress/ui/layoutpanel.ui"
        || rUIFile == u"modules/simpress/ui/masterpagepanel.ui"
        || rUIFile == u"modules/simpress/ui/masterpagepanelall.ui"
        || rUIFile == u"modules/simpress/ui/masterpagepanelrecent.ui"
        || rUIFile == u"modules/simpress/ui/navigatorpanel.ui"
        || rUIFile == u"modules/simpress/ui/sidebarslidebackground.ui"
        || rUIFile == u"modules/simpress/ui/slidetransitionspanel.ui"
        || rUIFile == u"modules/simpress/ui/tabledesignpanel.ui"
        // smath
        || rUIFile == u"modules/smath/ui/sidebarelements_math.ui"
        || rUIFile == u"modules/smath/ui/sidebarproperties_math.ui"
        // swriter
        || rUIFile == u"modules/swriter/ui/a11ycheckissuespanel.ui"
        || rUIFile == u"modules/swriter/ui/managechangessidebar.ui"
        || rUIFile == u"modules/swriter/ui/navigatorpanel.ui"
        || rUIFile == u"modules/swriter/ui/pagefooterpanel.ui"
        || rUIFile == u"modules/swriter/ui/pageformatpanel.ui"
        || rUIFile == u"modules/swriter/ui/pageheaderpanel.ui"
        || rUIFile == u"modules/swriter/ui/pagestylespanel.ui"
        || rUIFile == u"modules/swriter/ui/sidebarstylepresets.ui"
        || rUIFile == u"modules/swriter/ui/sidebartableedit.ui"
        || rUIFile == u"modules/swriter/ui/sidebartheme.ui"
        || rUIFile == u"modules/swriter/ui/sidebarwrap.ui"
        // sfx
        || rUIFile == u"sfx/ui/panel.ui"
        || rUIFile == u"sfx/ui/templatepanel.ui"
        // svx
        || rUIFile == u"svx/ui/defaultshapespanel.ui"
        || rUIFile == u"svx/ui/inspectortextpanel.ui"
        || rUIFile == u"svx/ui/mediaplayback.ui"
        || rUIFile == u"svx/ui/sidebararea.ui"
        || rUIFile == u"svx/ui/sidebareffect.ui"
        || rUIFile == u"svx/ui/sidebarempty.ui"
        || rUIFile == u"svx/ui/sidebarfontwork.ui"
        || rUIFile == u"svx/ui/sidebargallery.ui"
        || rUIFile == u"svx/ui/sidebargraphic.ui"
        || rUIFile == u"svx/ui/sidebarline.ui"
        || rUIFile == u"svx/ui/sidebarlists.ui"
        || rUIFile == u"svx/ui/sidebarparagraph.ui"
        || rUIFile == u"svx/ui/sidebarpossize.ui"
        || rUIFile == u"svx/ui/sidebarshadow.ui"
        || rUIFile == u"svx/ui/sidebarstylespanel.ui"
        || rUIFile == u"svx/ui/sidebartextpanel.ui";
}

bool isInterimBuilderEnabledForNotebookbar(std::u16string_view rUIFile)
{
    if (rUIFile == u"modules/scalc/ui/numberbox.ui"
        || rUIFile == u"svx/ui/fontnamebox.ui"
        || rUIFile == u"svx/ui/fontsizebox.ui"
        || rUIFile == u"svx/ui/stylespreview.ui")
    {
        return true;
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
