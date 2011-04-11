/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
            ScCursorRefEdit( ScAnyRefDlg* pParent, const ResId& rResId );
    void    SetCursorLinks( const Link& rUp, const Link& rDown );

protected:
    virtual void    KeyInput( const KeyEvent& rKEvt );
};


/// The dialog's content for a row, not yet parsed
struct ScOptConditionRow
{
    String  aLeftStr;
    sal_uInt16  nOperator;
    String  aRightStr;

    ScOptConditionRow() : nOperator(0) {}
    bool IsDefault() const { return aLeftStr.Len() == 0 && aRightStr.Len() == 0 && nOperator == 0; }
};

/// All settings from the dialog, saved with the DocShell for the next call
class ScOptSolverSave
{
    String  maObjective;
    sal_Bool    mbMax;
    sal_Bool    mbMin;
    sal_Bool    mbValue;
    String  maTarget;
    String  maVariable;
    std::vector<ScOptConditionRow> maConditions;
    String  maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

public:
            ScOptSolverSave( const String& rObjective, sal_Bool bMax, sal_Bool bMin, sal_Bool bValue,
                             const String& rTarget, const String& rVariable,
                             const std::vector<ScOptConditionRow>& rConditions,
                             const String& rEngine,
                             const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );

    const String&   GetObjective() const    { return maObjective; }
    sal_Bool            GetMax() const          { return mbMax; }
    sal_Bool            GetMin() const          { return mbMin; }
    sal_Bool            GetValue() const        { return mbValue; }
    const String&   GetTarget() const       { return maTarget; }
    const String&   GetVariable() const     { return maVariable; }
    const std::vector<ScOptConditionRow>& GetConditions() const { return maConditions; }
    const String&   GetEngine() const       { return maEngine; }
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
    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    FixedText       maFtObjectiveCell;
    formula::RefEdit       maEdObjectiveCell;
    formula::RefButton     maRBObjectiveCell;

    FixedText       maFtDirection;
    RadioButton     maRbMax;
    RadioButton     maRbMin;
    RadioButton     maRbValue;
    formula::RefEdit       maEdTargetValue;
    formula::RefButton     maRBTargetValue;

    FixedText       maFtVariableCells;
    formula::RefEdit       maEdVariableCells;
    formula::RefButton     maRBVariableCells;

    FixedLine       maFlConditions;

    FixedText       maFtCellRef;        // labels are together with controls for the first row
    ScCursorRefEdit maEdLeft1;
    formula::RefButton     maRBLeft1;
    FixedText       maFtOperator;
    ListBox         maLbOp1;
    FixedText       maFtConstraint;
    ScCursorRefEdit maEdRight1;
    formula::RefButton     maRBRight1;
    ImageButton     maBtnDel1;

    ScCursorRefEdit maEdLeft2;
    formula::RefButton     maRBLeft2;
    ListBox         maLbOp2;
    ScCursorRefEdit maEdRight2;
    formula::RefButton     maRBRight2;
    ImageButton     maBtnDel2;

    ScCursorRefEdit maEdLeft3;
    formula::RefButton     maRBLeft3;
    ListBox         maLbOp3;
    ScCursorRefEdit maEdRight3;
    formula::RefButton     maRBRight3;
    ImageButton     maBtnDel3;

    ScCursorRefEdit maEdLeft4;
    formula::RefButton     maRBLeft4;
    ListBox         maLbOp4;
    ScCursorRefEdit maEdRight4;
    formula::RefButton     maRBRight4;
    ImageButton     maBtnDel4;

    ScrollBar       maScrollBar;

    FixedLine       maFlButtons;

    PushButton      maBtnOpt;
    HelpButton      maBtnHelp;
    CancelButton    maBtnCancel;
    PushButton      maBtnSolve;

    String          maInputError;
    String          maConditionError;

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
    ImageButton*     mpDelButton[EDIT_ROW_COUNT];

    std::vector<ScOptConditionRow> maConditions;
    long            nScrollPos;

    com::sun::star::uno::Sequence<rtl::OUString> maImplNames;
    com::sun::star::uno::Sequence<rtl::OUString> maDescriptions;
    String          maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

    void    Init(const ScAddress& rCursorPos);
    bool    CallSolver();
    void    ReadConditions();
    void    ShowConditions();
    void    EnableButtons();
    bool    ParseRef( ScRange& rRange, const String& rInput, bool bAllowRange );
    bool    FindTimeout( sal_Int32& rTimeout );
    void    ShowError( bool bCondition, formula::RefEdit* pFocus );

    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( DelBtnHdl, PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, Control* );
    DECL_LINK( ScrollHdl, ScrollBar* );
    DECL_LINK( CursorUpHdl, ScCursorRefEdit* );
    DECL_LINK( CursorDownHdl, ScCursorRefEdit* );
    DECL_LINK( CondModifyHdl, Edit* );
    DECL_LINK( TargetModifyHdl, Edit* );
    DECL_LINK( SelectHdl, ListBox* );
};


class ScSolverProgressDialog : public ModelessDialog
{
    FixedText       maFtProgress;
    FixedText       maFtTime;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;

public:
    ScSolverProgressDialog( Window* pParent );
    ~ScSolverProgressDialog();

    void    HideTimeLimit();
    void    SetTimeLimit( sal_Int32 nSeconds );
};

class ScSolverNoSolutionDialog : public ModalDialog
{
    FixedText       maFtNoSolution;
    FixedText       maFtErrorText;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;

public:
    ScSolverNoSolutionDialog( Window* pParent, const String& rErrorText );
    ~ScSolverNoSolutionDialog();
};

class ScSolverSuccessDialog : public ModalDialog
{
    FixedText       maFtSuccess;
    FixedText       maFtResult;
    FixedText       maFtQuestion;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverSuccessDialog( Window* pParent, const String& rSolution );
    ~ScSolverSuccessDialog();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
