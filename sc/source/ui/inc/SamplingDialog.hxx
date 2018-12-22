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

#include <vcl/fixed.hxx>

class ScSamplingDialog : public ScAnyRefDlg
{
public:
    ScSamplingDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScSamplingDialog() override;
    virtual void    dispose() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual void    SetActive() override;
    virtual bool    Close() override;

private:
    // Widgets
    VclPtr<FixedText>          mpInputRangeLabel;
    VclPtr<formula::RefEdit>   mpInputRangeEdit;
    VclPtr<formula::RefButton> mpInputRangeButton;

    VclPtr<FixedText>          mpOutputRangeLabel;
    VclPtr<formula::RefEdit>   mpOutputRangeEdit;
    VclPtr<formula::RefButton> mpOutputRangeButton;

    VclPtr<NumericField>       mpSampleSize;
    VclPtr<NumericField>       mpPeriod;

    VclPtr<RadioButton>        mpRandomMethodRadio;
    VclPtr<CheckBox>           mpWithReplacement;
    VclPtr<CheckBox>           mpKeepOrder;
    VclPtr<RadioButton>        mpPeriodicMethodRadio;

    VclPtr<OKButton>           mpButtonOk;

    VclPtr<formula::RefEdit>   mpActiveEdit;

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

    void Init();
    void GetRangeFromSelection();
    void PerformSampling();
    sal_Int64 GetPopulationSize() const;
    void LimitSampleSizeAndPeriod();

    ScRange PerformRandomSampling(ScDocShell* pDocShell);
    ScRange PerformRandomSamplingKeepOrder(ScDocShell* pDocShell);
    ScRange PerformPeriodicSampling(ScDocShell* pDocShell);

    DECL_LINK( OkClicked, Button*, void );
    DECL_LINK( GetFocusHandler, Control&, void );
    DECL_LINK( LoseFocusHandler, Control&, void );
    DECL_LINK( SamplingSizeValueModified, Edit&, void );
    DECL_LINK( PeriodValueModified, Edit&, void );
    DECL_LINK( ToggleSamplingMethod, RadioButton&, void );
    DECL_LINK( RefInputModifyHandler, Edit&, void );
    DECL_LINK( CheckHdl, Button*, void );
    void ToggleSamplingMethod();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
