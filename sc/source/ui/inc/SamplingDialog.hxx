/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef SAMPLING_DIALOG_HXX
#define SAMPLING_DIALOG_HXX

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
        Window* pParent, ScViewData* pViewData );

    virtual ~ScSamplingDialog();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void        SetActive();
    virtual sal_Bool    Close();

private:
    // Widgets
    FixedText*          mpInputRangeLabel;
    formula::RefEdit*   mpInputRangeEdit;
    formula::RefButton* mpInputRangeButton;

    FixedText*          mpOutputRangeLabel;
    formula::RefEdit*   mpOutputRangeEdit;
    formula::RefButton* mpOutputRangeButton;

    PushButton*         mpButtonApply;
    OKButton*           mpButtonOk;
    CancelButton*       mpButtonCancel;

    //
    formula::RefEdit*   mpActiveEdit;

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

    DECL_LINK( OkClicked,        PushButton* );
    DECL_LINK( CancelClicked,    PushButton* );
    DECL_LINK( ApplyClicked,     PushButton* );
    DECL_LINK( GetFocusHandler,  Control* );
    DECL_LINK( LoseFocusHandler, void* );

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
