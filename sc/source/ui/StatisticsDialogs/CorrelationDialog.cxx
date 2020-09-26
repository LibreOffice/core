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
#include <CorrelationDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScCorrelationDialog::ScCorrelationDialog(
                        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                        weld::Window* pParent, ScViewData& rViewData ) :
    ScMatrixComparisonGenerator(
            pSfxBindings, pChildWindow, pParent, rViewData,
            "modules/scalc/ui/correlationdialog.ui", "CorrelationDialog")
{}

void ScCorrelationDialog::Close()
{
    DoClose(ScCorrelationDialogWrapper::GetChildWindowId());
}

OUString ScCorrelationDialog::getLabel()
{
    return ScResId(STR_CORRELATION_LABEL);
}

OUString ScCorrelationDialog::getTemplate()
{
    return "=CORREL(%VAR1%; %VAR2%)";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
