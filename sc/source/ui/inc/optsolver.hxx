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

#ifndef SC_OPTSOLVER_HXX
#define SC_OPTSOLVER_HXX

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

//============================================================================

class ScCursorRefEdit : public formula::RefEdit
{
    Link    maCursorUpLink;
    Link    maCursorDownLink;

public:
            ScCursorRefEdit( Window* pParent, Window *pLabel );
    void    SetCursorLinks( const Link& rUp, const Link& rDown );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt );
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
    sal_Bool    mbMax;
    sal_Bool    mbMin;
    sal_Bool    mbValue;
    OUString    maTarget;
    OUString    maVariable;
    std::vector<ScOptConditionRow> maConditions;
    OUString    maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

public:
            ScOptSolverSave( const OUString& rObjective, sal_Bool bMax, sal_Bool bMin, sal_Bool bValue,
                             const OUString& rTarget, const OUString& rVariable,
                             const std::vector<ScOptConditionRow>& rConditions,
                             const OUString& rEngine,
                             const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );

    const OUString&   GetObjective() const    { return maObjective; }
    sal_Bool          GetMax() const          { return mbMax; }
    sal_Bool          GetMin() const          { return mbMin; }
    sal_Bool          GetValue() const        { return mbValue; }
    const OUString&   GetTarget() const       { return maTarget; }
    const OUString&   GetVariable() const     { return maVariable; }
    const std::vector<ScOptConditionRow>& GetConditions() const { return maConditions; }
    const OUString&   GetEngine() const       { return maEngine; }
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& GetProperties() const
                                            { return maProperties; }
};

class ScOptSolverDlg : public ScAnyRefDlg
{
public:
                    ScOptSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScDocShell* pDocSh, ScAddress aCursorPos );
                    ~ScOptSolverDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    FixedText*       m_pFtObjectiveCell;
    formula::RefEdit*    m_pEdObjectiveCell;
    formula::RefButton*  m_pRBObjectiveCell;

    RadioButton*     m_pRbMax;
    RadioButton*     m_pRbMin;
    RadioButton*     m_pRbValue;
    formula::RefEdit*   m_pEdTargetValue;
    formula::RefButton* m_pRBTargetValue;

    FixedText*       m_pFtVariableCells;
    formula::RefEdit*   m_pEdVariableCells;
    formula::RefButton* m_pRBVariableCells;

    FixedText*       m_pFtCellRef; // labels are together with controls for the first row
    ScCursorRefEdit* m_pEdLeft1;
    formula::RefButton* m_pRBLeft1;
    FixedText*       m_pFtOperator;
    ListBox*         m_pLbOp1;
    FixedText*       m_pFtConstraint;
    ScCursorRefEdit* m_pEdRight1;
    formula::RefButton* m_pRBRight1;
    PushButton*      m_pBtnDel1;

    ScCursorRefEdit* m_pEdLeft2;
    formula::RefButton* m_pRBLeft2;
    ListBox*         m_pLbOp2;
    ScCursorRefEdit* m_pEdRight2;
    formula::RefButton* m_pRBRight2;
    PushButton*      m_pBtnDel2;

    ScCursorRefEdit* m_pEdLeft3;
    formula::RefButton* m_pRBLeft3;
    ListBox*         m_pLbOp3;
    ScCursorRefEdit* m_pEdRight3;
    formula::RefButton* m_pRBRight3;
    PushButton*      m_pBtnDel3;

    ScCursorRefEdit* m_pEdLeft4;
    formula::RefButton* m_pRBLeft4;
    ListBox*         m_pLbOp4;
    ScCursorRefEdit* m_pEdRight4;
    formula::RefButton* m_pRBRight4;
    PushButton*      m_pBtnDel4;

    ScrollBar*       m_pScrollBar;

    PushButton*      m_pBtnOpt;
    PushButton*      m_pBtnCancel;
    PushButton*      m_pBtnSolve;

    OUString        maInputError;
    OUString        maConditionError;

    ScDocShell*     mpDocShell;
    ScDocument*     mpDoc;
    const SCTAB     mnCurTab;
    formula::RefEdit*      mpEdActive;
    bool            mbDlgLostFocus;

    static const sal_uInt16 EDIT_ROW_COUNT = 4;
    ScCursorRefEdit* mpLeftEdit[EDIT_ROW_COUNT];
    formula::RefButton*     mpLeftButton[EDIT_ROW_COUNT];
    ScCursorRefEdit* mpRightEdit[EDIT_ROW_COUNT];
    formula::RefButton*     mpRightButton[EDIT_ROW_COUNT];
    ListBox*         mpOperator[EDIT_ROW_COUNT];
    PushButton*      mpDelButton[EDIT_ROW_COUNT];

    std::vector<ScOptConditionRow> maConditions;
    long            nScrollPos;

    com::sun::star::uno::Sequence<OUString> maImplNames;
    com::sun::star::uno::Sequence<OUString> maDescriptions;
    OUString        maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

    void    Init(const ScAddress& rCursorPos);
    bool    CallSolver();
    void    ReadConditions();
    void    ShowConditions();
    void    EnableButtons();
    bool    ParseRef( ScRange& rRange, const OUString& rInput, bool bAllowRange );
    bool    FindTimeout( sal_Int32& rTimeout );
    void    ShowError( bool bCondition, formula::RefEdit* pFocus );

    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( DelBtnHdl, PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, void* );
    DECL_LINK(ScrollHdl, void *);
    DECL_LINK( CursorUpHdl, ScCursorRefEdit* );
    DECL_LINK( CursorDownHdl, ScCursorRefEdit* );
    DECL_LINK( CondModifyHdl, void* );
    DECL_LINK( TargetModifyHdl, void* );
    DECL_LINK( SelectHdl, void* );
};


class ScSolverProgressDialog : public ModelessDialog
{
    FixedText* m_pFtTime;

public:
    ScSolverProgressDialog( Window* pParent );

    void    HideTimeLimit();
    void    SetTimeLimit( sal_Int32 nSeconds );
};

class ScSolverNoSolutionDialog : public ModalDialog
{
    FixedText* m_pFtErrorText;

public:
    ScSolverNoSolutionDialog(Window* pParent, const OUString& rErrorText);
};

class ScSolverSuccessDialog : public ModalDialog
{
    FixedText* m_pFtResult;
    PushButton* m_pBtnOk;
    PushButton* m_pBtnCancel;

    DECL_LINK(ClickHdl, PushButton*);

public:
    ScSolverSuccessDialog( Window* pParent, const OUString& rSolution );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
