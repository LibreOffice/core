/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

#include <SparklineGroup.hxx>
#include <SparklineAttributes.hxx>

namespace sc
{
/** Dialog to change input data range for a sparkline  */
class SparklineDataRangeDialog : public ScAnyRefDlgController
{
private:
    ScViewData& mrViewData;
    ScDocument& mrDocument;

    std::shared_ptr<sc::Sparkline> mpSparkline;

    ScRange maDataRange;

    formula::RefEdit* mpActiveEdit;
    bool mbDialogLostFocus;

    std::unique_ptr<weld::Button> mxButtonOk;
    std::unique_ptr<weld::Button> mxButtonCancel;

    std::unique_ptr<weld::Label> mxDataRangeLabel;
    std::unique_ptr<formula::RefEdit> mxDataRangeEdit;
    std::unique_ptr<formula::RefButton> mxDataRangeButton;

    DECL_LINK(ButtonClicked, weld::Button&, void);
    DECL_LINK(EditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(ButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(LoseEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(LoseButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(RefInputModifyHandler, formula::RefEdit&, void);

    void setupValues();

    void perform();
    bool checkValidInputOutput();

public:
    SparklineDataRangeDialog(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                             weld::Window* pWindow, ScViewData& rViewData);
    virtual ~SparklineDataRangeDialog() override;

    void SetReference(const ScRange& rRef, ScDocument& rDocument) override;
    void SetActive() override;
    void Close() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
