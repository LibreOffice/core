/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_SAMPLINGDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SAMPLINGDIALOG_HXX

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

class ScSamplingDialog : public ScAnyRefDlgController
{
public:
    ScSamplingDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData& rViewData );

    virtual ~ScSamplingDialog() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    formula::RefEdit* mpActiveEdit;

    // Data
    ScViewData&               mViewData;
    const ScDocument&         mDocument;

    ScRange             mInputRange;
    ScAddress::Details  mAddressDetails;
    ScAddress           mOutputAddress;

    ScAddress           mCurrentAddress;

    sal_Int64                 mnLastSampleSizeValue;
    sal_Int64                 mnLastPeriodValue;

    bool                mDialogLostFocus;

    // Widgets
    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::RefButton> mxInputRangeButton;

    std::unique_ptr<weld::Label> mxOutputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxOutputRangeEdit;
    std::unique_ptr<formula::RefButton> mxOutputRangeButton;

    std::unique_ptr<weld::SpinButton> mxSampleSize;
    std::unique_ptr<weld::SpinButton> mxPeriod;

    std::unique_ptr<weld::RadioButton> mxRandomMethodRadio;
    std::unique_ptr<weld::CheckButton> mxWithReplacement;
    std::unique_ptr<weld::CheckButton> mxKeepOrder;
    std::unique_ptr<weld::RadioButton> mxPeriodicMethodRadio;

    std::unique_ptr<weld::Button> mxButtonOk;
    std::unique_ptr<weld::Button> mxButtonCancel;

    void Init();
    void GetRangeFromSelection();
    void PerformSampling();
    sal_Int64 GetPopulationSize() const;
    void LimitSampleSizeAndPeriod();

    ScRange PerformRandomSampling(ScDocShell* pDocShell);
    ScRange PerformRandomSamplingKeepOrder(ScDocShell* pDocShell);
    ScRange PerformPeriodicSampling(ScDocShell* pDocShell);

    DECL_LINK( ButtonClicked, weld::Button&, void );
    DECL_LINK( GetEditFocusHandler, formula::RefEdit&, void );
    DECL_LINK( GetButtonFocusHandler, formula::RefButton&, void );
    DECL_LINK( LoseEditFocusHandler, formula::RefEdit&, void );
    DECL_LINK( LoseButtonFocusHandler, formula::RefButton&, void );
    DECL_LINK( SamplingSizeValueModified, weld::SpinButton&, void );
    DECL_LINK( PeriodValueModified, weld::SpinButton&, void );
    DECL_LINK( ToggleSamplingMethod, weld::ToggleButton&, void );
    DECL_LINK( RefInputModifyHandler, formula::RefEdit&, void );
    DECL_LINK( CheckHdl, weld::Button&, void );
    void ToggleSamplingMethod();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
