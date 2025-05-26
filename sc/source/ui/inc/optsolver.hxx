/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <address.hxx>
#include "anyrefdg.hxx"
#include "docsh.hxx"
#include <SolverSettings.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/table/CellAddress.hpp>

#include <string_view>
#include <vector>

namespace com::sun::star {
    namespace beans { struct PropertyValue; }
}

class ScCursorRefEdit : public formula::RefEdit
{
    Link<ScCursorRefEdit&,void>  maCursorUpLink;
    Link<ScCursorRefEdit&,void>  maCursorDownLink;

public:
    ScCursorRefEdit(std::unique_ptr<weld::Entry> xEntry);
    void    SetCursorLinks( const Link<ScCursorRefEdit&,void>& rUp, const Link<ScCursorRefEdit&,void>& rDown );

protected:
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
};

class ScSolverOptionsDialog;

class ScOptSolverDlg : public ScAnyRefDlgController
{
public:
    ScOptSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                 ScDocShell& rDocSh, const ScAddress& aCursorPos );
    virtual ~ScOptSolverDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    OUString        maInputError;
    OUString        maConditionError;

    ScDocShell&     mrDocShell;
    ScDocument&     mrDoc;
    const SCTAB     mnCurTab;
    bool            mbDlgLostFocus;

    std::vector<sc::ModelConstraint> m_aConditions;
    tools::Long            nScrollPos;

    css::uno::Sequence<OUString> maImplNames;
    css::uno::Sequence<OUString> maDescriptions;
    OUString        maEngine;
    css::uno::Sequence<css::beans::PropertyValue> maProperties;

    static const sal_uInt16 EDIT_ROW_COUNT = 4;
    ScCursorRefEdit* mpLeftEdit[EDIT_ROW_COUNT];
    formula::RefButton* mpLeftButton[EDIT_ROW_COUNT];
    ScCursorRefEdit* mpRightEdit[EDIT_ROW_COUNT];
    formula::RefButton* mpRightButton[EDIT_ROW_COUNT];
    weld::ComboBox* mpOperator[EDIT_ROW_COUNT];
    weld::Button* mpDelButton[EDIT_ROW_COUNT];

    formula::RefEdit* mpEdActive;

    std::unique_ptr<weld::Label> m_xFtObjectiveCell;
    std::unique_ptr<formula::RefEdit>    m_xEdObjectiveCell;
    std::unique_ptr<formula::RefButton>  m_xRBObjectiveCell;

    std::unique_ptr<weld::RadioButton> m_xRbMax;
    std::unique_ptr<weld::RadioButton> m_xRbMin;
    std::unique_ptr<weld::RadioButton> m_xRbValue;
    std::unique_ptr<formula::RefEdit>   m_xEdTargetValue;
    std::unique_ptr<formula::RefButton> m_xRBTargetValue;

    std::unique_ptr<weld::Label> m_xFtVariableCells;
    std::unique_ptr<formula::RefEdit>   m_xEdVariableCells;
    std::unique_ptr<formula::RefButton> m_xRBVariableCells;

    std::unique_ptr<weld::Label> m_xFtCellRef; // labels are together with controls for the first row
    std::unique_ptr<ScCursorRefEdit> m_xEdLeft1;
    std::unique_ptr<formula::RefButton> m_xRBLeft1;
    std::unique_ptr<weld::ComboBox> m_xLbOp1;
    std::unique_ptr<weld::Label> m_xFtConstraint;
    std::unique_ptr<ScCursorRefEdit> m_xEdRight1;
    std::unique_ptr<formula::RefButton> m_xRBRight1;
    std::unique_ptr<weld::Button> m_xBtnDel1;

    std::unique_ptr<ScCursorRefEdit> m_xEdLeft2;
    std::unique_ptr<formula::RefButton> m_xRBLeft2;
    std::unique_ptr<weld::ComboBox> m_xLbOp2;
    std::unique_ptr<ScCursorRefEdit> m_xEdRight2;
    std::unique_ptr<formula::RefButton> m_xRBRight2;
    std::unique_ptr<weld::Button> m_xBtnDel2;

    std::unique_ptr<ScCursorRefEdit> m_xEdLeft3;
    std::unique_ptr<formula::RefButton> m_xRBLeft3;
    std::unique_ptr<weld::ComboBox> m_xLbOp3;
    std::unique_ptr<ScCursorRefEdit> m_xEdRight3;
    std::unique_ptr<formula::RefButton> m_xRBRight3;
    std::unique_ptr<weld::Button> m_xBtnDel3;

    std::unique_ptr<ScCursorRefEdit> m_xEdLeft4;
    std::unique_ptr<formula::RefButton> m_xRBLeft4;
    std::unique_ptr<weld::ComboBox> m_xLbOp4;
    std::unique_ptr<ScCursorRefEdit> m_xEdRight4;
    std::unique_ptr<formula::RefButton> m_xRBRight4;
    std::unique_ptr<weld::Button> m_xBtnDel4;

    std::unique_ptr<weld::ScrolledWindow> m_xScrollBar;

    std::unique_ptr<weld::Button> m_xBtnOpt;
    std::unique_ptr<weld::Button> m_xBtnClose;
    std::unique_ptr<weld::Button> m_xBtnSolve;
    std::unique_ptr<weld::Button> m_xBtnResetAll;

    std::unique_ptr<weld::Label> m_xResultFT;
    std::unique_ptr<weld::Widget> m_xContents;

    std::shared_ptr<ScSolverOptionsDialog> m_xOptDlg;
    std::shared_ptr<sc::SolverSettings> m_pSolverSettings;

    void    Init(const ScAddress& rCursorPos);
    bool    CallSolver();
    void    ReadConditions();
    void    ShowConditions();
    void    EnableButtons();
    bool    ParseRef( ScRange& rRange, const OUString& rInput, bool bAllowRange );
    bool    FindTimeout( sal_Int32& rTimeout );
    void    ShowError( bool bCondition, formula::RefEdit* pFocus );
    void    LoadSolverSettings();
    void    SaveSolverSettings();
    bool    IsEngineAvailable(std::u16string_view sEngineName);

    static sc::ConstraintOperator OperatorIndexToConstraintOperator(sal_Int32 nIndex);

    // Return the string representation of a css::table::CellAddress
    OUString GetCellStrAddress(css::table::CellAddress aUnoAddress);

    DECL_LINK( BtnHdl, weld::Button&, void );
    DECL_LINK( DelBtnHdl, weld::Button&, void );
    DECL_LINK( GetEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( GetButtonFocusHdl, formula::RefButton&, void );
    DECL_LINK( GetFocusHdl, weld::Widget&, void );
    DECL_LINK( LoseEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( LoseButtonFocusHdl, formula::RefButton&, void );
    DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void);
    DECL_LINK( CursorUpHdl, ScCursorRefEdit&, void );
    DECL_LINK( CursorDownHdl, ScCursorRefEdit&, void );
    DECL_LINK( CondModifyHdl, formula::RefEdit&, void );
    DECL_LINK( TargetModifyHdl, formula::RefEdit&, void );
    DECL_LINK( SelectHdl, weld::ComboBox&, void );
};

class ScSolverProgressDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Label> m_xFtTime;

public:
    ScSolverProgressDialog(weld::Window* pParent);
    virtual ~ScSolverProgressDialog() override;

    void    HideTimeLimit();
    void    SetTimeLimit( sal_Int32 nSeconds );
};

class ScSolverNoSolutionDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Label> m_xFtErrorText;

public:
    ScSolverNoSolutionDialog(weld::Window* pParent, const OUString& rErrorText);
    virtual ~ScSolverNoSolutionDialog() override;
};

class ScSolverSuccessDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Label> m_xFtResult;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    DECL_LINK(ClickHdl, weld::Button&, void);

public:
    ScSolverSuccessDialog(weld::Window* pParent, std::u16string_view rSolution);
    virtual ~ScSolverSuccessDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
