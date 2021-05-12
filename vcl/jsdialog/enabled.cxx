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
        || rUIFile == "modules/scalc/ui/textimportcsv.ui")
    {
        return true;
    }

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
