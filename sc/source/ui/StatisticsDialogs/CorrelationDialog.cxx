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

#include "CorrelationDialog.hxx"

ScCorrelationDialog::ScCorrelationDialog(
                        SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                        vcl::Window* pParent, ScViewData* pViewData ) :
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
    return SC_RESSTR(STR_CORRELATION_LABEL);
}

const OUString ScCorrelationDialog::getTemplate()
{
    return OUString("=CORREL(%VAR1%; %VAR2%)");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
