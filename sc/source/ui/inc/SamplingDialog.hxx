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

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

class ScSamplingDialog : public ScAnyRefDlg
{
public:
    ScSamplingDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData );

    virtual ~ScSamplingDialog();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

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
    VclPtr<RadioButton>        mpPeriodicMethodRadio;

    VclPtr<OKButton>           mpButtonOk;

    VclPtr<formula::RefEdit>   mpActiveEdit;

    // Data
    ScViewData*         mViewData;
    ScDocument*         mDocument;

    ScRange             mInputRange;
    ScAddress::Details  mAddressDetails;
    ScAddress           mOutputAddress;

    ScAddress           mCurrentAddress;

    bool                mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();
    void PerformSampling();

    ScRange PerformRandomSampling(ScDocShell* pDocShell);
    ScRange PerformPeriodicSampling(ScDocShell* pDocShell);

    DECL_LINK_TYPED( OkClicked, Button*, void );
    DECL_LINK_TYPED( GetFocusHandler, Control&, void );
    DECL_LINK_TYPED( LoseFocusHandler, Control&, void );
    DECL_LINK( SamplingSizeValueModified, void* );
    DECL_LINK_TYPED( ToggleSamplingMethod, RadioButton&, void );
    DECL_LINK( RefInputModifyHandler, void* );
    void ToggleSamplingMethod();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
