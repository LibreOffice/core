/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jsdialog/enabled.hxx>

namespace jsdialog
{
bool isBuilderEnabled(const OUString& rUIFile, bool bMobile)
{
    // mobile only dialogs
    if (bMobile)
    {
        if (rUIFile == "modules/swriter/ui/wordcount-mobile.ui"
            || rUIFile == "svx/ui/findreplacedialog-mobile.ui"
            || rUIFile == "modules/swriter/ui/watermarkdialog.ui"
            || rUIFile == "modules/scalc/ui/validationdialog.ui"
            || rUIFile == "modules/scalc/ui/validationcriteriapage.ui"
            || rUIFile == "modules/scalc/ui/validationhelptabpage-mobile.ui"
            || rUIFile == "modules/scalc/ui/erroralerttabpage-mobile.ui"
            || rUIFile == "modules/scalc/ui/validationdialog.ui")
        {
            return true;
        }
    }

    // dialogs
    if (rUIFile == "modules/scalc/ui/pivottablelayoutdialog.ui"
        || rUIFile == "modules/scalc/ui/selectsource.ui"
        || rUIFile == "modules/scalc/ui/managenamesdialog.ui"
        || rUIFile == "modules/scalc/ui/definename.ui"
        || rUIFile == "modules/scalc/ui/correlationdialog.ui"
        || rUIFile == "modules/scalc/ui/samplingdialog.ui"
        || rUIFile == "modules/scalc/ui/descriptivestatisticsdialog.ui"
        || rUIFile == "modules/scalc/ui/analysisofvariancedialog.ui"
        || rUIFile == "modules/scalc/ui/covariancedialog.ui"
        || rUIFile == "modules/scalc/ui/exponentialsmoothingdialog.ui"
        || rUIFile == "modules/scalc/ui/movingaveragedialog.ui"
        || rUIFile == "modules/scalc/ui/regressiondialog.ui"
        || rUIFile == "modules/scalc/ui/ttestdialog.ui"
        || rUIFile == "modules/scalc/ui/ttestdialog.ui"
        || rUIFile == "modules/scalc/ui/ztestdialog.ui"
        || rUIFile == "modules/scalc/ui/chisquaretestdialog.ui"
        || rUIFile == "modules/scalc/ui/fourieranalysisdialog.ui"
        || rUIFile == "modules/scalc/ui/datafielddialog.ui"
        || rUIFile == "modules/scalc/ui/pivotfielddialog.ui"
        || rUIFile == "modules/scalc/ui/datafieldoptionsdialog.ui"
        || rUIFile == "svx/ui/fontworkgallerydialog.ui"
        || rUIFile == "cui/ui/macroselectordialog.ui" || rUIFile == "uui/ui/macrowarnmedium.ui"
        || rUIFile == "modules/scalc/ui/textimportcsv.ui"
        || rUIFile == "xmlsec/ui/digitalsignaturesdialog.ui")
    {
        return true;
    }

    return false;
}

bool isBuilderEnabledForPopup(const OUString& rUIFile)
{
    if (rUIFile == "svx/ui/colorwindow.ui" || rUIFile == "modules/scalc/ui/floatinglinestyle.ui"
        || rUIFile == "svx/ui/textcharacterspacingcontrol.ui"
        || rUIFile == "svx/ui/currencywindow.ui"
        || rUIFile == "modules/scalc/ui/floatingborderstyle.ui"
        || rUIFile == "svx/ui/textunderlinecontrol.ui" || rUIFile == "svx/ui/numberingwindow.ui"
        || rUIFile == "svx/ui/paralinespacingcontrol.ui"
        || rUIFile == "svx/ui/floatinglineproperty.ui" || rUIFile == "svx/ui/floatinglinestyle.ui"
        || rUIFile == "svx/ui/floatinglineend.ui" || rUIFile == "svx/ui/fontworkalignmentcontrol.ui"
        || rUIFile == "svx/ui/fontworkcharacterspacingcontrol.ui"
        || rUIFile == "svx/ui/floatingareastyle.ui")
        return true;

    return false;
}

bool isBuilderEnabledForSidebar(const OUString& rUIFile)
{
    if (rUIFile == "sfx/ui/panel.ui" || rUIFile == "svx/ui/sidebartextpanel.ui"
        || rUIFile == "svx/ui/sidebarstylespanel.ui" || rUIFile == "svx/ui/sidebarparagraph.ui"
        || rUIFile == "svx/ui/sidebarlists.ui" || rUIFile == "svx/ui/sidebararea.ui"
        || rUIFile == "svx/ui/sidebarfontwork.ui" || rUIFile == "svx/ui/sidebarshadow.ui"
        || rUIFile == "svx/ui/sidebareffect.ui" || rUIFile == "svx/ui/sidebargraphic.ui"
        || rUIFile == "svx/ui/sidebarline.ui" || rUIFile == "svx/ui/sidebarpossize.ui"
        || rUIFile == "svx/ui/defaultshapespanel.ui" || rUIFile == "svx/ui/mediaplayback.ui"
        || rUIFile == "svx/ui/sidebargallery.ui" || rUIFile == "sfx/ui/templatepanel.ui"
        || rUIFile == "svx/ui/sidebarempty.ui" || rUIFile == "modules/schart/ui/sidebarelements.ui"
        || rUIFile == "modules/schart/ui/sidebartype.ui"
        || rUIFile == "modules/schart/ui/sidebarseries.ui"
        || rUIFile == "modules/schart/ui/sidebaraxis.ui"
        || rUIFile == "modules/schart/ui/sidebarerrorbar.ui"
        || rUIFile == "modules/simpress/ui/customanimationspanel.ui"
        || rUIFile == "modules/simpress/ui/layoutpanel.ui"
        || rUIFile == "modules/simpress/ui/masterpagepanel.ui"
        || rUIFile == "modules/simpress/ui/masterpagepanelall.ui"
        || rUIFile == "modules/simpress/ui/masterpagepanelrecent.ui"
        || rUIFile == "modules/simpress/ui/slidetransitionspanel.ui"
        || rUIFile == "modules/simpress/ui/tabledesignpanel.ui"
        || rUIFile == "modules/simpress/ui/navigatorpanel.ui"
        || rUIFile == "modules/simpress/ui/sidebarslidebackground.ui"
        || rUIFile == "modules/scalc/ui/sidebaralignment.ui"
        || rUIFile == "modules/scalc/ui/sidebarcellappearance.ui"
        || rUIFile == "modules/scalc/ui/sidebarnumberformat.ui"
        || rUIFile == "modules/scalc/ui/navigatorpanel.ui"
        || rUIFile == "modules/scalc/ui/functionpanel.ui"
        || rUIFile == "modules/swriter/ui/pagestylespanel.ui"
        || rUIFile == "modules/swriter/ui/pageformatpanel.ui"
        || rUIFile == "modules/swriter/ui/pageheaderpanel.ui"
        || rUIFile == "modules/swriter/ui/pagefooterpanel.ui"
        || rUIFile == "modules/swriter/ui/sidebarwrap.ui"
        || rUIFile == "modules/swriter/ui/navigatorpanel.ui"
        || rUIFile == "modules/swriter/ui/managechangessidebar.ui"
        || rUIFile == "svx/ui/inspectortextpanel.ui"
        || rUIFile == "modules/swriter/ui/sidebarstylepresets.ui"
        || rUIFile == "modules/swriter/ui/sidebartheme.ui"
        || rUIFile == "modules/swriter/ui/sidebartableedit.ui")
        return true;

    return false;
}

bool isInterimBuilderEnabledForNotebookbar(const OUString& rUIFile)
{
    if (rUIFile == "svx/ui/stylespreview.ui" || rUIFile == "modules/scalc/ui/numberbox.ui")
    {
        return true;
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
