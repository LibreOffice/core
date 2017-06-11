/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_STATISTICSINPUTOUTPUTDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_STATISTICSINPUTOUTPUTDIALOG_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>

class ScStatisticsInputOutputDialog : public ScAnyRefDlg
{
public:
    enum GroupedBy {
        BY_COLUMN,
        BY_ROW
    };

    ScStatisticsInputOutputDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        vcl::Window* pParent, ScViewData* pViewData,
        const OUString& rID, const OUString& rUIXMLDescription );

    virtual ~ScStatisticsInputOutputDialog() override;
    virtual void        dispose() override;

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual void        SetActive() override;

protected:
    void CalculateInputAndWriteToOutput();

    virtual ScRange ApplyOutput(ScDocShell* pDocShell) = 0;
    virtual const char* GetUndoNameId() = 0;

    // Widgets
    VclPtr<FixedText>          mpInputRangeLabel;
    VclPtr<formula::RefEdit>   mpInputRangeEdit;
    VclPtr<formula::RefButton> mpInputRangeButton;

    VclPtr<FixedText>          mpOutputRangeLabel;
    VclPtr<formula::RefEdit>   mpOutputRangeEdit;
    VclPtr<formula::RefButton> mpOutputRangeButton;

    VclPtr<RadioButton>        mpGroupByColumnsRadio;
    VclPtr<RadioButton>        mpGroupByRowsRadio;

    // Data
    ScViewData*         mViewData;
    ScDocument*         mDocument;

    ScRange             mInputRange;
    ScAddress::Details  mAddressDetails;
    ScAddress           mOutputAddress;
    GroupedBy           mGroupedBy;

    static ScRangeList MakeColumnRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);
    static ScRangeList MakeRowRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);

private:
    // Widgets
    VclPtr<OKButton>           mpButtonOk;

    VclPtr<formula::RefEdit>   mpActiveEdit;
    ScAddress           mCurrentAddress;
    bool                mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK( GroupByChanged, RadioButton&, void );
    DECL_LINK( OkClicked, Button*, void );
    DECL_LINK( GetFocusHandler,  Control&, void );
    DECL_LINK( LoseFocusHandler, Control&, void );
    DECL_LINK( RefInputModifyHandler, Edit&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
