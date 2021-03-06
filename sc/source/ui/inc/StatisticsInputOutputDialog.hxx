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

class ScStatisticsInputOutputDialog : public ScAnyRefDlgController
{
public:
    enum GroupedBy {
        BY_COLUMN,
        BY_ROW
    };

    ScStatisticsInputOutputDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData& rViewData,
        const OUString& rUIXMLDescription,
        const OString& rID);

    virtual ~ScStatisticsInputOutputDialog() override;

    virtual void        SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual void        SetActive() override;

protected:
    void CalculateInputAndWriteToOutput();

    virtual ScRange ApplyOutput(ScDocShell* pDocShell) = 0;
    virtual const char* GetUndoNameId() = 0;
    virtual bool InputRangesValid();
    void ValidateDialogInput();

    // Widgets
    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::RefButton> mxInputRangeButton;

    std::unique_ptr<weld::Label> mxOutputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxOutputRangeEdit;
    std::unique_ptr<formula::RefButton> mxOutputRangeButton;

    std::unique_ptr<weld::RadioButton> mxGroupByColumnsRadio;
    std::unique_ptr<weld::RadioButton> mxGroupByRowsRadio;

    // Data
    ScViewData&         mViewData;
    ScDocument&         mDocument;

    ScRange             mInputRange;
    ScAddress::Details  mAddressDetails;
    ScAddress           mOutputAddress;
    GroupedBy           mGroupedBy;

    static ScRangeList MakeColumnRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);
    static ScRangeList MakeRowRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);

private:
    // Widgets
    std::unique_ptr<weld::Button>       mxButtonOk;
    std::unique_ptr<weld::Button>       mxButtonCancel;

    formula::RefEdit*      mpActiveEdit;
    ScAddress              mCurrentAddress;
    bool                   mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK( GroupByChanged, weld::ToggleButton&, void );
    DECL_LINK( ButtonClicked, weld::Button&, void );
    DECL_LINK( GetEditFocusHandler,  formula::RefEdit&, void );
    DECL_LINK( GetButtonFocusHandler,  formula::RefButton&, void );
    DECL_LINK( LoseEditFocusHandler, formula::RefEdit&, void );
    DECL_LINK( LoseButtonFocusHandler, formula::RefButton&, void );
    DECL_LINK( RefInputModifyHandler, formula::RefEdit&, void );
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
