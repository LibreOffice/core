/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_STATISTICSTWOVARIABLEDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_STATISTICSTWOVARIABLEDIALOG_HXX

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
        vcl::Window* pParent, ScViewData* pViewData,
        const OUString& rID, const OUString& rUIXMLDescription );

    virtual ~ScStatisticsTwoVariableDialog();
    virtual void        dispose() override;

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual void        SetActive() override;

protected:
    void CalculateInputAndWriteToOutput();

    virtual ScRange ApplyOutput(ScDocShell* pDocShell) = 0;
    virtual sal_Int16 GetUndoNameId() = 0;

    // Widgets
    VclPtr<FixedText>          mpVariable1RangeLabel;
    VclPtr<formula::RefEdit>   mpVariable1RangeEdit;
    VclPtr<formula::RefButton> mpVariable1RangeButton;

    VclPtr<FixedText>          mpVariable2RangeLabel;
    VclPtr<formula::RefEdit>   mpVariable2RangeEdit;
    VclPtr<formula::RefButton> mpVariable2RangeButton;

    VclPtr<FixedText>          mpOutputRangeLabel;
    VclPtr<formula::RefEdit>   mpOutputRangeEdit;
    VclPtr<formula::RefButton> mpOutputRangeButton;

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
    VclPtr<OKButton>           mpButtonOk;

    VclPtr<RadioButton>        mpGroupByColumnsRadio;
    VclPtr<RadioButton>        mpGroupByRowsRadio;

    VclPtr<formula::RefEdit>   mpActiveEdit;
    ScAddress           mCurrentAddress;
    bool                mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK_TYPED( GroupByChanged, RadioButton&, void );
    DECL_LINK_TYPED( OkClicked, Button*, void );
    DECL_LINK_TYPED( GetFocusHandler, Control&, void );
    DECL_LINK_TYPED( LoseFocusHandler, Control&, void );
    DECL_LINK_TYPED( RefInputModifyHandler, Edit&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
