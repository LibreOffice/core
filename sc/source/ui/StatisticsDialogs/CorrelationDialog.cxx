/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "docsh.hxx"
#include "reffact.hxx"
#include "strload.hxx"
#include "StatisticsDialogs.hrc"

#include "CorrelationDialog.hxx"

namespace
{
    static const OUString strCorrelationTemplate("=CORREL(%VAR1%; %VAR2%)");
}

ScCorrelationDialog::ScCorrelationDialog(
                        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                        Window* pParent, ScViewData* pViewData ) :
    ScMatrixComparisonGenerator(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "CorrelationDialog", "modules/scalc/ui/correlationdialog.ui" )
{}

bool ScCorrelationDialog::Close()
{
    return DoClose( ScCorrelationDialogWrapper::GetChildWindowId() );
}

const OUString ScCorrelationDialog::getLabel()
{
    return SC_STRLOAD(RID_STATISTICS_DLGS, STR_CORRELATION_LABEL);
}

const OUString ScCorrelationDialog::getTemplate()
{
    return strCorrelationTemplate;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
