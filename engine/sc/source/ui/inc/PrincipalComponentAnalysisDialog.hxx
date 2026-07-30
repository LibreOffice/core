/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

class ScDocShell;

class ScPrincipalComponentAnalysisDialog : public ScAnyRefDlgController
{
public:
    ScPrincipalComponentAnalysisDialog(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                                       ScViewData& rViewData);

    virtual ~ScPrincipalComponentAnalysisDialog() override;

    virtual void SetReference(const ScRange& rRef, ScDocument& rDoc) override;
    virtual void SetActive() override;
    virtual void Close() override;

private:
    ScViewData& mrViewData;
    ScDocument& mrDocument;

    ScRange maInputRange;
    ScAddress::Details maAddressDetails;

    // The part of the input range that holds numbers, so the header row is
    // left out when the labels box is ticked.
    ScRange maDataRange;
    bool mbWithLabels;

    bool mbDialogLostFocus;

    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::RefButton> mxInputRangeButton;
    std::unique_ptr<weld::CheckButton> mxWithLabelsCheckBox;
    std::unique_ptr<weld::Label> mxErrorMessage;
    std::unique_ptr<weld::Button> mxButtonCalculate;
    std::unique_ptr<weld::Button> mxButtonCancel;

    void Init();
    void GetRangeFromSelection();
    void ValidateDialogInput();

    /** Fills maDataRange and mbWithLabels, and puts a message in the error
        label when the input cannot be analysed. */
    bool InputIsValid();
    OUString GetOutputSheetName() const;
    void Calculate();
    ScRange WriteOutput(ScDocShell& rDocShell, SCTAB nOutputTab);

    DECL_LINK(ButtonClicked, weld::Button&, void);
    DECL_LINK(CheckBoxToggled, weld::Toggleable&, void);
    DECL_LINK(GetEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(GetButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(LoseEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(LoseButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(RefInputModifyHandler, formula::RefEdit&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
