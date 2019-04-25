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

class ScStatisticsTwoVariableDialog : public ScAnyRefDlgController
{
public:
    enum GroupedBy {
        BY_COLUMN,
        BY_ROW
    };

    ScStatisticsTwoVariableDialog(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData* pViewData,
        const OUString& rUIXMLDescription, const OString& rID);

    virtual ~ScStatisticsTwoVariableDialog() override;

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual void        SetActive() override;

protected:
    void CalculateInputAndWriteToOutput();

    virtual ScRange ApplyOutput(ScDocShell* pDocShell) = 0;
    virtual const char* GetUndoNameId() = 0;
    virtual bool InputRangesValid();
    void ValidateDialogInput();

    // Widgets
    std::unique_ptr<weld::Label> mxVariable1RangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxVariable1RangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxVariable1RangeButton;

    std::unique_ptr<weld::Label> mxVariable2RangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxVariable2RangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxVariable2RangeButton;

    std::unique_ptr<weld::Label> mxOutputRangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxOutputRangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxOutputRangeButton;

    // Data
    ScViewData* const         mViewData;
    ScDocument* const         mDocument;

    ScRange                   mVariable1Range;
    ScRange                   mVariable2Range;

    ScAddress::Details const  mAddressDetails;
    ScAddress                 mOutputAddress;
    GroupedBy                 mGroupedBy;

private:
    // Widgets
    std::unique_ptr<weld::Button> mxButtonOk;

    std::unique_ptr<weld::RadioButton> mxGroupByColumnsRadio;
    std::unique_ptr<weld::RadioButton> mxGroupByRowsRadio;

    formula::WeldRefEdit*      mpActiveEdit;
    ScAddress const            mCurrentAddress;
    bool                       mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK( GroupByChanged, weld::ToggleButton&, void );
    DECL_LINK( OkClicked, weld::Button&, void );
    DECL_LINK( GetEditFocusHandler, formula::WeldRefEdit&, void );
    DECL_LINK( GetButtonFocusHandler, formula::WeldRefButton&, void );
    DECL_LINK( LoseEditFocusHandler, formula::WeldRefEdit&, void );
    DECL_LINK( LoseButtonFocusHandler, formula::WeldRefButton&, void );
    DECL_LINK( RefInputModifyHandler, formula::WeldRefEdit&, void );
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
