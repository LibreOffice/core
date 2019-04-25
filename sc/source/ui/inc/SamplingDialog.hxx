/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

#include <vcl/fixed.hxx>

class ScSamplingDialog : public ScAnyRefDlgController
{
public:
    ScSamplingDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData* pViewData );

    virtual ~ScSamplingDialog() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    formula::WeldRefEdit* mpActiveEdit;

    // Data
    ScViewData* const         mViewData;
    ScDocument* const         mDocument;

    ScRange                   mInputRange;
    ScAddress::Details const  mAddressDetails;
    ScAddress                 mOutputAddress;

    ScAddress const           mCurrentAddress;

    sal_Int64                 mnLastSampleSizeValue;
    sal_Int64                 mnLastPeriodValue;

    bool                mDialogLostFocus;

    // Widgets
    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxInputRangeButton;

    std::unique_ptr<weld::Label> mxOutputRangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxOutputRangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxOutputRangeButton;

    std::unique_ptr<weld::SpinButton> mxSampleSize;
    std::unique_ptr<weld::SpinButton> mxPeriod;

    std::unique_ptr<weld::RadioButton> mxRandomMethodRadio;
    std::unique_ptr<weld::CheckButton> mxWithReplacement;
    std::unique_ptr<weld::CheckButton> mxKeepOrder;
    std::unique_ptr<weld::RadioButton> mxPeriodicMethodRadio;

    std::unique_ptr<weld::Button> mxButtonOk;

    void Init();
    void GetRangeFromSelection();
    void PerformSampling();
    sal_Int64 GetPopulationSize() const;
    void LimitSampleSizeAndPeriod();

    ScRange PerformRandomSampling(ScDocShell* pDocShell);
    ScRange PerformRandomSamplingKeepOrder(ScDocShell* pDocShell);
    ScRange PerformPeriodicSampling(ScDocShell* pDocShell);

    DECL_LINK( OkClicked, weld::Button&, void );
    DECL_LINK( GetEditFocusHandler, formula::WeldRefEdit&, void );
    DECL_LINK( GetButtonFocusHandler, formula::WeldRefButton&, void );
    DECL_LINK( LoseEditFocusHandler, formula::WeldRefEdit&, void );
    DECL_LINK( LoseButtonFocusHandler, formula::WeldRefButton&, void );
    DECL_LINK( SamplingSizeValueModified, weld::SpinButton&, void );
    DECL_LINK( PeriodValueModified, weld::SpinButton&, void );
    DECL_LINK( ToggleSamplingMethod, weld::ToggleButton&, void );
    DECL_LINK( RefInputModifyHandler, formula::WeldRefEdit&, void );
    DECL_LINK( CheckHdl, weld::Button&, void );
    void ToggleSamplingMethod();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
