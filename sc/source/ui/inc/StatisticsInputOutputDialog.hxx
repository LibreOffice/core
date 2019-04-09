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

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

#include <vcl/fixed.hxx>

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
    virtual bool InputRangesValid();
    void ValidateDialogInput();

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
    ScViewData* const         mViewData;
    ScDocument* const         mDocument;

    ScRange                   mInputRange;
    ScAddress::Details const  mAddressDetails;
    ScAddress                 mOutputAddress;
    GroupedBy                 mGroupedBy;

    static ScRangeList MakeColumnRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);
    static ScRangeList MakeRowRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);

private:
    // Widgets
    VclPtr<OKButton>           mpButtonOk;

    VclPtr<formula::RefEdit>   mpActiveEdit;
    ScAddress const            mCurrentAddress;
    bool                       mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK( GroupByChanged, RadioButton&, void );
    DECL_LINK( OkClicked, Button*, void );
    DECL_LINK( GetFocusHandler,  Control&, void );
    DECL_LINK( LoseFocusHandler, Control&, void );
    DECL_LINK( RefInputModifyHandler, Edit&, void );
};

class ScStatisticsInputOutputDialogController : public ScAnyRefDlgController
{
public:
    enum GroupedBy {
        BY_COLUMN,
        BY_ROW
    };

    ScStatisticsInputOutputDialogController(
        SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, ScViewData* pViewData,
        const OUString& rUIXMLDescription,
        const OString& rID);

    virtual ~ScStatisticsInputOutputDialogController() override;

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual void        SetActive() override;

protected:
    void CalculateInputAndWriteToOutput();

    virtual ScRange ApplyOutput(ScDocShell* pDocShell) = 0;
    virtual const char* GetUndoNameId() = 0;
    virtual bool InputRangesValid();
    void ValidateDialogInput();

    // Widgets
    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxInputRangeButton;

    std::unique_ptr<weld::Label> mxOutputRangeLabel;
    std::unique_ptr<formula::WeldRefEdit> mxOutputRangeEdit;
    std::unique_ptr<formula::WeldRefButton> mxOutputRangeButton;

    std::unique_ptr<weld::RadioButton> mxGroupByColumnsRadio;
    std::unique_ptr<weld::RadioButton> mxGroupByRowsRadio;

    // Data
    ScViewData* const         mViewData;
    ScDocument* const         mDocument;

    ScRange                   mInputRange;
    ScAddress::Details const  mAddressDetails;
    ScAddress                 mOutputAddress;
    GroupedBy                 mGroupedBy;

    static ScRangeList MakeColumnRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);
    static ScRangeList MakeRowRangeList(SCTAB aTab, ScAddress const & aStart, ScAddress const & aEnd);

private:
    // Widgets
    std::unique_ptr<weld::Button>       mxButtonOk;

    formula::WeldRefEdit*      mpActiveEdit;
    ScAddress const            mCurrentAddress;
    bool                       mDialogLostFocus;

    void Init();
    void GetRangeFromSelection();

    DECL_LINK( GroupByChanged, weld::ToggleButton&, void );
    DECL_LINK( OkClicked, weld::Button&, void );
    DECL_LINK( GetEditFocusHandler,  formula::WeldRefEdit&, void );
    DECL_LINK( GetButtonFocusHandler,  formula::WeldRefButton&, void );
    DECL_LINK( LoseEditFocusHandler, formula::WeldRefEdit&, void );
    DECL_LINK( LoseButtonFocusHandler, formula::WeldRefButton&, void );
    DECL_LINK( RefInputModifyHandler, formula::WeldRefEdit&, void );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
