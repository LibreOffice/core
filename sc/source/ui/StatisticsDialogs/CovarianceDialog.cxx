/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <reffact.hxx>
#include <CovarianceDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScCovarianceDialog::ScCovarianceDialog(
                        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                        weld::Window* pParent, ScViewData* pViewData ) :
    ScMatrixComparisonGenerator(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "modules/scalc/ui/covariancedialog.ui", "CovarianceDialog")
{}

const char* ScCovarianceDialog::GetUndoNameId()
{
    return STR_COVARIANCE_UNDO_NAME;
}

void ScCovarianceDialog::Close()
{
    DoClose( ScCovarianceDialogWrapper::GetChildWindowId() );
}

const OUString ScCovarianceDialog::getLabel()
{
    return ScResId(STR_COVARIANCE_LABEL);
}

const OUString ScCovarianceDialog::getTemplate()
{
    return OUString("=COVAR(%VAR1%; %VAR2%)");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
