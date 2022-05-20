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

class ColorListBox;

namespace sc
{
/** Dialog to create or edit sparkline group attributes */
class SparklineDialog : public ScAnyRefDlgController
{
private:
    ScViewData& mrViewData;
    ScDocument& mrDocument;

    ScRange maInputRange;
    ScRange maOutputRange;

    formula::RefEdit* mpActiveEdit;
    bool mbDialogLostFocus;

    std::unique_ptr<weld::Button> mxButtonOk;
    std::unique_ptr<weld::Button> mxButtonCancel;

    std::unique_ptr<weld::Frame> mxFrameData;

    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::RefButton> mxInputRangeButton;

    std::unique_ptr<weld::Label> mxOutputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxOutputRangeEdit;
    std::unique_ptr<formula::RefButton> mxOutputRangeButton;

    std::unique_ptr<ColorListBox> mxColorSeries;
    std::unique_ptr<ColorListBox> mxColorNegative;
    std::unique_ptr<ColorListBox> mxColorMarker;
    std::unique_ptr<ColorListBox> mxColorHigh;
    std::unique_ptr<ColorListBox> mxColorLow;
    std::unique_ptr<ColorListBox> mxColorFirst;
    std::unique_ptr<ColorListBox> mxColorLast;

    std::unique_ptr<weld::CheckButton> mxCheckButtonNegative;
    std::unique_ptr<weld::CheckButton> mxCheckButtonMarker;
    std::unique_ptr<weld::CheckButton> mxCheckButtonHigh;
    std::unique_ptr<weld::CheckButton> mxCheckButtonLow;
    std::unique_ptr<weld::CheckButton> mxCheckButtonFirst;
    std::unique_ptr<weld::CheckButton> mxCheckButtonLast;

    std::unique_ptr<weld::SpinButton> mxSpinLineWidth;
    std::unique_ptr<weld::ComboBox> mxType;

    std::unique_ptr<weld::CheckButton> mxCheckDisplayXAxis;
    std::unique_ptr<weld::CheckButton> mxCheckDisplayHidden;
    std::unique_ptr<weld::CheckButton> mxCheckRightToLeft;

    std::unique_ptr<weld::ComboBox> mxDisplayEmptyGap;

    std::unique_ptr<weld::ComboBox> mxComboMinAxisType;
    std::unique_ptr<weld::ComboBox> mxComboMaxAxisType;

    std::unique_ptr<weld::FormattedSpinButton> mxSpinCustomMin;
    std::unique_ptr<weld::FormattedSpinButton> mxSpinCustomMax;

    DECL_LINK(ButtonClicked, weld::Button&, void);
    DECL_LINK(EditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(ButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(LoseEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(LoseButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(RefInputModifyHandler, formula::RefEdit&, void);
    DECL_LINK(ToggleHandler, weld::Toggleable&, void);
    DECL_LINK(SelectSparklineType, weld::ComboBox&, void);
    DECL_LINK(ComboValueChanged, weld::ComboBox&, void);
    DECL_LINK(SpinLineWidthChanged, weld::SpinButton&, void);
    DECL_LINK(SpinCustomChanged, weld::FormattedSpinButton&, void);

    std::shared_ptr<sc::SparklineGroup> mpSparklineGroup;
    sc::SparklineAttributes maAttributes;

    bool mbEditMode;

    void setupValues();
    void setInputSelection();

    void perform();
    bool checkValidInputOutput();

public:
    SparklineDialog(SfxBindings* pBindings, SfxChildWindow* pChildWindow, weld::Window* pWindow,
                    ScViewData& rViewData);
    virtual ~SparklineDialog() override;

    virtual void SetReference(const ScRange& rRef, ScDocument& rDocument) override;
    virtual void SetActive() override;
    virtual void Close() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
