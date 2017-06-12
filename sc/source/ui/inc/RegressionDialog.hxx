/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_REGRESSIONDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_REGRESSIONDIALOG_HXX

#include "StatisticsTwoVariableDialog.hxx"

class ScRegressionDialog : public ScStatisticsTwoVariableDialog
{
    VclPtr<CheckBox> mpLinearCheckBox;
    VclPtr<CheckBox> mpLogarithmicCheckBox;
    VclPtr<CheckBox> mpPowerCheckBox;

public:
    ScRegressionDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScRegressionDialog() override;

    virtual bool Close() override;

protected:
    void dispose() override;
    virtual const char* GetUndoNameId() override;
    virtual ScRange ApplyOutput(ScDocShell* pDocShell) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
