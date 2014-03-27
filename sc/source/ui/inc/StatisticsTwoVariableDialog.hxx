/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef STATISTICS_TWO_VARIABLE_DIALOG_HXX
#define STATISTICS_TWO_VARIABLE_DIALOG_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

class ScStatisticsTwoVariableDialog : public ScAnyRefDlg
{
public:
    enum GroupedBy {
        BY_COLUMN,
        BY_ROW
    };

    ScStatisticsTwoVariableDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        Window* pParent, ScViewData* pViewData,
        const OString& rID, const OUString& rUIXMLDescription );

    virtual ~ScStatisticsTwoVariableDialog();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual void        SetActive() SAL_OVERRIDE;

protected:
    virtual void CalculateInputAndWriteToOutput();

    virtual ScRange ApplyOutput(ScDocShell* pDocShell) = 0;
    virtual sal_Int16 GetUndoNameId() = 0;

    // Widgets
    FixedText*          mpVariable1RangeLabel;
    formula::RefEdit*   mpVariable1RangeEdit;
    formula::RefButton* mpVariable1RangeButton;

    FixedText*          mpVariable2RangeLabel;
    formula::RefEdit*   mpVariable2RangeEdit;
    formula::RefButton* mpVariable2RangeButton;

    FixedText*          mpOutputRangeLabel;
    formula::RefEdit*   mpOutputRangeEdit;
    formula::RefButton* mpOutputRangeButton;

    // Data
    ScViewData*         mViewData;
    ScDocument*         mDocument;

    ScRange             mVariable1Range;
    ScRange             mVariable2Range;

    ScAddress::Details  mAddressDetails;
    ScAddress           mOutputAddress;
    GroupedBy           mGroupedBy;

private:
    // Widgets
    PushButton*         mpButtonApply;
    OKButton*           mpButtonOk;
    CloseButton*        mpButtonClose;
    RadioButton*        mpGroupByColumnsRadio;
    RadioButton*        mpGroupByRowsRadio;

    formula::RefEdit*   mpActiveEdit;
    ScAddress           mCurrentAddress;
    bool                mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK( GroupByChanged,   void* );
    DECL_LINK( OkClicked,        PushButton* );
    DECL_LINK( CloseClicked,     PushButton* );
    DECL_LINK( ApplyClicked,     PushButton* );
    DECL_LINK( GetFocusHandler,  Control* );
    DECL_LINK( LoseFocusHandler, void* );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
