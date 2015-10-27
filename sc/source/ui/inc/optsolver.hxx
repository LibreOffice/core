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

#ifndef INCLUDED_SC_SOURCE_UI_INC_OPTSOLVER_HXX
#define INCLUDED_SC_SOURCE_UI_INC_OPTSOLVER_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

class ScCursorRefEdit : public formula::RefEdit
{
    Link<ScCursorRefEdit&,void>  maCursorUpLink;
    Link<ScCursorRefEdit&,void>  maCursorDownLink;

public:
            ScCursorRefEdit( vcl::Window* pParent, vcl::Window *pLabel );
    void    SetCursorLinks( const Link<ScCursorRefEdit&,void>& rUp, const Link<ScCursorRefEdit&,void>& rDown );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
};

/// The dialog's content for a row, not yet parsed
struct ScOptConditionRow
{
    OUString    aLeftStr;
    sal_uInt16  nOperator;
    OUString    aRightStr;

    ScOptConditionRow() : nOperator(0) {}
    bool IsDefault() const { return aLeftStr.isEmpty() && aRightStr.isEmpty() && nOperator == 0; }
};

/// All settings from the dialog, saved with the DocShell for the next call
class ScOptSolverSave
{
    OUString    maObjective;
    bool        mbMax;
    bool        mbMin;
    bool        mbValue;
    OUString    maTarget;
    OUString    maVariable;
    std::vector<ScOptConditionRow> maConditions;
    OUString    maEngine;
    css::uno::Sequence<css::beans::PropertyValue> maProperties;

public:
            ScOptSolverSave( const OUString& rObjective, bool bMax, bool bMin, bool bValue,
                             const OUString& rTarget, const OUString& rVariable,
                             const std::vector<ScOptConditionRow>& rConditions,
                             const OUString& rEngine,
                             const css::uno::Sequence<css::beans::PropertyValue>& rProperties );

    const OUString&   GetObjective() const    { return maObjective; }
    bool              GetMax() const          { return mbMax; }
    bool              GetMin() const          { return mbMin; }
    bool              GetValue() const        { return mbValue; }
    const OUString&   GetTarget() const       { return maTarget; }
    const OUString&   GetVariable() const     { return maVariable; }
    const std::vector<ScOptConditionRow>& GetConditions() const { return maConditions; }
    const OUString&   GetEngine() const       { return maEngine; }
    const css::uno::Sequence<css::beans::PropertyValue>& GetProperties() const
                                            { return maProperties; }
};

class ScOptSolverDlg : public ScAnyRefDlg
{
public:
                    ScOptSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                 ScDocShell* pDocSh, ScAddress aCursorPos );
                    virtual ~ScOptSolverDlg();
    virtual void    dispose() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) override;
    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual bool    Close() override;

private:
    VclPtr<FixedText>       m_pFtObjectiveCell;
    VclPtr<formula::RefEdit>    m_pEdObjectiveCell;
    VclPtr<formula::RefButton>  m_pRBObjectiveCell;

    VclPtr<RadioButton>     m_pRbMax;
    VclPtr<RadioButton>     m_pRbMin;
    VclPtr<RadioButton>     m_pRbValue;
    VclPtr<formula::RefEdit>   m_pEdTargetValue;
    VclPtr<formula::RefButton> m_pRBTargetValue;

    VclPtr<FixedText>       m_pFtVariableCells;
    VclPtr<formula::RefEdit>   m_pEdVariableCells;
    VclPtr<formula::RefButton> m_pRBVariableCells;

    VclPtr<FixedText>       m_pFtCellRef; // labels are together with controls for the first row
    VclPtr<ScCursorRefEdit> m_pEdLeft1;
    VclPtr<formula::RefButton> m_pRBLeft1;
    VclPtr<FixedText>       m_pFtOperator;
    VclPtr<ListBox>         m_pLbOp1;
    VclPtr<FixedText>       m_pFtConstraint;
    VclPtr<ScCursorRefEdit> m_pEdRight1;
    VclPtr<formula::RefButton> m_pRBRight1;
    VclPtr<PushButton>      m_pBtnDel1;

    VclPtr<ScCursorRefEdit> m_pEdLeft2;
    VclPtr<formula::RefButton> m_pRBLeft2;
    VclPtr<ListBox>         m_pLbOp2;
    VclPtr<ScCursorRefEdit> m_pEdRight2;
    VclPtr<formula::RefButton> m_pRBRight2;
    VclPtr<PushButton>      m_pBtnDel2;

    VclPtr<ScCursorRefEdit> m_pEdLeft3;
    VclPtr<formula::RefButton> m_pRBLeft3;
    VclPtr<ListBox>         m_pLbOp3;
    VclPtr<ScCursorRefEdit> m_pEdRight3;
    VclPtr<formula::RefButton> m_pRBRight3;
    VclPtr<PushButton>      m_pBtnDel3;

    VclPtr<ScCursorRefEdit> m_pEdLeft4;
    VclPtr<formula::RefButton> m_pRBLeft4;
    VclPtr<ListBox>         m_pLbOp4;
    VclPtr<ScCursorRefEdit> m_pEdRight4;
    VclPtr<formula::RefButton> m_pRBRight4;
    VclPtr<PushButton>      m_pBtnDel4;

    VclPtr<ScrollBar>       m_pScrollBar;

    VclPtr<PushButton>      m_pBtnOpt;
    VclPtr<PushButton>      m_pBtnCancel;
    VclPtr<PushButton>      m_pBtnSolve;

    OUString        maInputError;
    OUString        maConditionError;

    ScDocShell*     mpDocShell;
    ScDocument&     mrDoc;
    const SCTAB     mnCurTab;
    VclPtr<formula::RefEdit>      mpEdActive;
    bool            mbDlgLostFocus;

    static const sal_uInt16 EDIT_ROW_COUNT = 4;
    VclPtr<ScCursorRefEdit>        mpLeftEdit[EDIT_ROW_COUNT];
    VclPtr<formula::RefButton>     mpLeftButton[EDIT_ROW_COUNT];
    VclPtr<ScCursorRefEdit>        mpRightEdit[EDIT_ROW_COUNT];
    VclPtr<formula::RefButton>     mpRightButton[EDIT_ROW_COUNT];
    VclPtr<ListBox>                mpOperator[EDIT_ROW_COUNT];
    VclPtr<PushButton>             mpDelButton[EDIT_ROW_COUNT];

    std::vector<ScOptConditionRow> maConditions;
    long            nScrollPos;

    css::uno::Sequence<OUString> maImplNames;
    css::uno::Sequence<OUString> maDescriptions;
    OUString        maEngine;
    css::uno::Sequence<css::beans::PropertyValue> maProperties;

    void    Init(const ScAddress& rCursorPos);
    bool    CallSolver();
    void    ReadConditions();
    void    ShowConditions();
    void    EnableButtons();
    bool    ParseRef( ScRange& rRange, const OUString& rInput, bool bAllowRange );
    bool    FindTimeout( sal_Int32& rTimeout );
    void    ShowError( bool bCondition, formula::RefEdit* pFocus );

    DECL_LINK_TYPED( BtnHdl, Button*, void );
    DECL_LINK_TYPED( DelBtnHdl, Button*, void );
    DECL_LINK_TYPED( GetFocusHdl, Control&, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void);
    DECL_LINK_TYPED( CursorUpHdl, ScCursorRefEdit&, void );
    DECL_LINK_TYPED( CursorDownHdl, ScCursorRefEdit&, void );
    DECL_LINK_TYPED( CondModifyHdl, Edit&, void );
    DECL_LINK_TYPED( TargetModifyHdl, Edit&, void );
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
};

class ScSolverProgressDialog : public ModelessDialog
{
    VclPtr<FixedText> m_pFtTime;

public:
    ScSolverProgressDialog( vcl::Window* pParent );
    virtual ~ScSolverProgressDialog();
    virtual void dispose() override;

    void    HideTimeLimit();
    void    SetTimeLimit( sal_Int32 nSeconds );
};

class ScSolverNoSolutionDialog : public ModalDialog
{
    VclPtr<FixedText> m_pFtErrorText;

public:
    ScSolverNoSolutionDialog(vcl::Window* pParent, const OUString& rErrorText);
    virtual ~ScSolverNoSolutionDialog();
    virtual void dispose() override;
};

class ScSolverSuccessDialog : public ModalDialog
{
    VclPtr<FixedText> m_pFtResult;
    VclPtr<PushButton> m_pBtnOk;
    VclPtr<PushButton> m_pBtnCancel;

    DECL_LINK_TYPED(ClickHdl, Button*, void);

public:
    ScSolverSuccessDialog( vcl::Window* pParent, const OUString& rSolution );
    virtual ~ScSolverSuccessDialog();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
