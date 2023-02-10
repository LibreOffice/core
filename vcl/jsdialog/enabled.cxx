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
        if (// scalc
            rUIFile == u"modules/scalc/ui/erroralerttabpage-mobile.ui"
            || rUIFile == u"modules/scalc/ui/validationcriteriapage.ui"
            || rUIFile == u"modules/scalc/ui/validationdialog.ui"
            || rUIFile == u"modules/scalc/ui/validationhelptabpage-mobile.ui"
            // swriter
            || rUIFile == u"modules/swriter/ui/watermarkdialog.ui"
            || rUIFile == u"modules/swriter/ui/wordcount-mobile.ui"
            // svx
            || rUIFile == u"svx/ui/findreplacedialog-mobile.ui")
        {
            return true;
        }
    }

    if (// cui
        rUIFile == u"cui/ui/formatnumberdialog.ui"
        || rUIFile == u"cui/ui/macroselectordialog.ui"
        || rUIFile == u"cui/ui/numberingformatpage.ui"
        || rUIFile == u"cui/ui/password.ui"
        || rUIFile == u"cui/ui/splitcellsdialog.ui"
        || rUIFile == u"cui/ui/similaritysearchdialog.ui"
        || rUIFile == u"cui/ui/widgettestdialog.ui"
        // scalc
        || rUIFile == u"modules/scalc/ui/analysisofvariancedialog.ui"
        || rUIFile == u"modules/scalc/ui/chisquaretestdialog.ui"
        || rUIFile == u"modules/scalc/ui/colwidthdialog.ui"
        || rUIFile == u"modules/scalc/ui/correlationdialog.ui"
        || rUIFile == u"modules/scalc/ui/covariancedialog.ui"
        || rUIFile == u"modules/scalc/ui/datafielddialog.ui"
        || rUIFile == u"modules/scalc/ui/datafieldoptionsdialog.ui"
        || rUIFile == u"modules/scalc/ui/definename.ui"
        || rUIFile == u"modules/scalc/ui/deletecells.ui"
        || rUIFile == u"modules/scalc/ui/deletecontents.ui"
        || rUIFile == u"modules/scalc/ui/descriptivestatisticsdialog.ui"
        || rUIFile == u"modules/scalc/ui/exponentialsmoothingdialog.ui"
        || rUIFile == u"modules/scalc/ui/fourieranalysisdialog.ui"
        || rUIFile == u"modules/scalc/ui/groupdialog.ui"
        || rUIFile == u"modules/scalc/ui/insertcells.ui"
        || rUIFile == u"modules/scalc/ui/managenamesdialog.ui"
        || rUIFile == u"modules/scalc/ui/movingaveragedialog.ui"
        || rUIFile == u"modules/scalc/ui/optimalcolwidthdialog.ui"
        || rUIFile == u"modules/scalc/ui/optimalrowheightdialog.ui"
        || rUIFile == u"modules/scalc/ui/pivotfielddialog.ui"
        || rUIFile == u"modules/scalc/ui/pivottablelayoutdialog.ui"
        || rUIFile == u"modules/scalc/ui/regressiondialog.ui"
        || rUIFile == u"modules/scalc/ui/rowheightdialog.ui"
        || rUIFile == u"modules/scalc/ui/samplingdialog.ui"
        || rUIFile == u"modules/scalc/ui/selectsource.ui"
        || rUIFile == u"modules/scalc/ui/textimportcsv.ui"
        || rUIFile == u"modules/scalc/ui/ttestdialog.ui"
        || rUIFile == u"modules/scalc/ui/ungroupdialog.ui"
        || rUIFile == u"modules/scalc/ui/ztestdialog.ui"
        // swriter
        || rUIFile == u"modules/swriter/ui/captionoptions.ui"
        || rUIFile == u"modules/swriter/ui/contentcontroldlg.ui"
        || rUIFile == u"modules/swriter/ui/contentcontrollistitemdlg.ui"
        || rUIFile == u"modules/swriter/ui/insertcaption.ui"
        || rUIFile == u"modules/swriter/ui/linenumbering.ui"
        || rUIFile == u"modules/swriter/ui/splittable.ui"
        || rUIFile == u"modules/swriter/ui/translationdialog.ui"
        || rUIFile == u"modules/swriter/ui/pagenumberdlg.ui"
        || rUIFile == u"modules/swriter/ui/dropdownfielddialog.ui"
        // sfx
        || rUIFile == u"sfx/ui/cmisinfopage.ui"
        || rUIFile == u"sfx/ui/custominfopage.ui"
        || rUIFile == u"sfx/ui/descriptioninfopage.ui"
        || rUIFile == u"sfx/ui/documentinfopage.ui"
        || rUIFile == u"sfx/ui/documentpropertiesdialog.ui"
        || rUIFile == u"sfx/ui/editdurationdialog.ui"
        || rUIFile == u"sfx/ui/linefragment.ui"
        || rUIFile == u"sfx/ui/password.ui"
        // svx
        || rUIFile == u"svx/ui/accessibilitycheckdialog.ui"
        || rUIFile == u"svx/ui/accessibilitycheckentry.ui"
        || rUIFile == u"svx/ui/findreplacedialog.ui"
        || rUIFile == u"svx/ui/fontworkgallerydialog.ui"
        // uui
        || rUIFile == u"uui/ui/macrowarnmedium.ui"
        // xmlsec
        || rUIFile == u"xmlsec/ui/certgeneral.ui"
        || rUIFile == u"xmlsec/ui/certpage.ui"
        || rUIFile == u"xmlsec/ui/digitalsignaturesdialog.ui"
        || rUIFile == u"xmlsec/ui/viewcertdialog.ui"
        || rUIFile == u"filter/ui/pdfoptionsdialog.ui"
        || rUIFile == u"filter/ui/pdfgeneralpage.ui"
        || rUIFile == u"filter/ui/pdfviewpage.ui"
        || rUIFile == u"filter/ui/pdfuserinterfacepage.ui"
        || rUIFile == u"filter/ui/pdfsecuritypage.ui"
        || rUIFile == u"filter/ui/pdflinkspage.ui"
        || rUIFile == u"filter/ui/warnpdfdialog.ui"
        || rUIFile == u"filter/ui/pdfsignpage.ui"
        || rUIFile == u"writerperfect/ui/exportepub.ui"
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
    if (// scalc
        rUIFile == u"modules/scalc/ui/functionpanel.ui"
        //|| rUIFile == u"modules/scalc/ui/navigatorpanel.ui"
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
        //|| rUIFile == u"modules/simpress/ui/navigatorpanel.ui"
        || rUIFile == u"modules/simpress/ui/sidebarslidebackground.ui"
        || rUIFile == u"modules/simpress/ui/slidetransitionspanel.ui"
        || rUIFile == u"modules/simpress/ui/tabledesignpanel.ui"
        // smath
        || rUIFile == u"modules/smath/ui/sidebarelements_math.ui"
        || rUIFile == u"modules/smath/ui/sidebarproperties_math.ui"
        // swriter
        || rUIFile == u"modules/swriter/ui/managechangessidebar.ui"
        //|| rUIFile == u"modules/swriter/ui/navigatorpanel.ui"
        || rUIFile == u"modules/swriter/ui/pageformatpanel.ui"
        || rUIFile == u"modules/swriter/ui/pagefooterpanel.ui"
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
        || rUIFile == u"svx/ui/sidebartextpanel.ui")
        return true;

    return false;
}

bool isInterimBuilderEnabledForNotebookbar(std::u16string_view rUIFile)
{
    if (rUIFile == u"modules/scalc/ui/numberbox.ui" || rUIFile == u"svx/ui/stylespreview.ui")
    {
        return true;
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
