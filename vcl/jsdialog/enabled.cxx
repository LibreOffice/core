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
bool isBuilderEnabled(std::u16string_view rUIFile, bool bMobile)
{
    if (bMobile)
    {
        if (rUIFile == u"modules/swriter/ui/wordcount-mobile.ui"
            || rUIFile == u"svx/ui/findreplacedialog-mobile.ui"
            || rUIFile == u"modules/swriter/ui/watermarkdialog.ui"
            || rUIFile == u"modules/scalc/ui/validationdialog.ui"
            || rUIFile == u"modules/scalc/ui/validationcriteriapage.ui"
            || rUIFile == u"modules/scalc/ui/validationhelptabpage-mobile.ui"
            || rUIFile == u"modules/scalc/ui/erroralerttabpage-mobile.ui"
            || rUIFile == u"modules/scalc/ui/validationdialog.ui")
        {
            return true;
        }
    }

    if (rUIFile == u"modules/scalc/ui/pivottablelayoutdialog.ui"
        || rUIFile == u"modules/scalc/ui/selectsource.ui"
        || rUIFile == u"modules/scalc/ui/managenamesdialog.ui"
        || rUIFile == u"modules/scalc/ui/definename.ui"
        || rUIFile == u"modules/scalc/ui/correlationdialog.ui"
        || rUIFile == u"modules/scalc/ui/samplingdialog.ui"
        || rUIFile == u"modules/scalc/ui/descriptivestatisticsdialog.ui"
        || rUIFile == u"modules/scalc/ui/analysisofvariancedialog.ui"
        || rUIFile == u"modules/scalc/ui/covariancedialog.ui"
        || rUIFile == u"modules/scalc/ui/exponentialsmoothingdialog.ui"
        || rUIFile == u"modules/scalc/ui/movingaveragedialog.ui"
        || rUIFile == u"modules/scalc/ui/regressiondialog.ui"
        || rUIFile == u"modules/scalc/ui/ttestdialog.ui"
        || rUIFile == u"modules/scalc/ui/ttestdialog.ui"
        || rUIFile == u"modules/scalc/ui/ztestdialog.ui"
        || rUIFile == u"modules/scalc/ui/chisquaretestdialog.ui"
        || rUIFile == u"modules/scalc/ui/fourieranalysisdialog.ui"
        || rUIFile == u"modules/scalc/ui/datafielddialog.ui"
        || rUIFile == u"modules/scalc/ui/pivotfielddialog.ui"
        || rUIFile == u"modules/scalc/ui/datafieldoptionsdialog.ui"
        || rUIFile == u"svx/ui/fontworkgallerydialog.ui"
        || rUIFile == u"cui/ui/macroselectordialog.ui" || rUIFile == u"uui/ui/macrowarnmedium.ui"
        || rUIFile == u"modules/scalc/ui/textimportc.ui")
    {
        return true;
    }

    return false;
}

bool isInterimBuilderEnabledForNotebookbar(std::u16string_view rUIFile)
{
    if (rUIFile == u"svx/ui/stylespreview.ui" || rUIFile == u"modules/scalc/ui/numberbox.ui")
    {
        return true;
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
