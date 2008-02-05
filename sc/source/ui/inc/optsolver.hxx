/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optsolver.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-05 15:46:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_OPTSOLVER_HXX
#define SC_OPTSOLVER_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <vector>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

//============================================================================

class ScCursorRefEdit : public ScRefEdit
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
    USHORT  nOperator;
    String  aRightStr;

    ScOptConditionRow() : nOperator(0) {}
    bool IsDefault() const { return aLeftStr.Len() == 0 && aRightStr.Len() == 0 && nOperator == 0; }
};

/// All settings from the dialog, saved with the DocShell for the next call
class ScOptSolverSave
{
    String  maObjective;
    BOOL    mbMax;
    BOOL    mbMin;
    BOOL    mbValue;
    String  maTarget;
    String  maVariable;
    std::vector<ScOptConditionRow> maConditions;
    String  maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

public:
            ScOptSolverSave( const String& rObjective, BOOL bMax, BOOL bMin, BOOL bValue,
                             const String& rTarget, const String& rVariable,
                             const std::vector<ScOptConditionRow>& rConditions,
                             const String& rEngine,
                             const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );

    const String&   GetObjective() const    { return maObjective; }
    BOOL            GetMax() const          { return mbMax; }
    BOOL            GetMin() const          { return mbMin; }
    BOOL            GetValue() const        { return mbValue; }
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
    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();
    virtual BOOL    Close();

private:
    FixedText       maFtObjectiveCell;
    ScRefEdit       maEdObjectiveCell;
    ScRefButton     maRBObjectiveCell;

    FixedText       maFtDirection;
    RadioButton     maRbMax;
    RadioButton     maRbMin;
    RadioButton     maRbValue;
    ScRefEdit       maEdTargetValue;
    ScRefButton     maRBTargetValue;

    FixedText       maFtVariableCells;
    ScRefEdit       maEdVariableCells;
    ScRefButton     maRBVariableCells;

    FixedLine       maFlConditions;

    FixedText       maFtCellRef;        // labels are together with controls for the first row
    ScCursorRefEdit maEdLeft1;
    ScRefButton     maRBLeft1;
    FixedText       maFtOperator;
    ListBox         maLbOp1;
    FixedText       maFtConstraint;
    ScCursorRefEdit maEdRight1;
    ScRefButton     maRBRight1;
    ImageButton     maBtnDel1;

    ScCursorRefEdit maEdLeft2;
    ScRefButton     maRBLeft2;
    ListBox         maLbOp2;
    ScCursorRefEdit maEdRight2;
    ScRefButton     maRBRight2;
    ImageButton     maBtnDel2;

    ScCursorRefEdit maEdLeft3;
    ScRefButton     maRBLeft3;
    ListBox         maLbOp3;
    ScCursorRefEdit maEdRight3;
    ScRefButton     maRBRight3;
    ImageButton     maBtnDel3;

    ScCursorRefEdit maEdLeft4;
    ScRefButton     maRBLeft4;
    ListBox         maLbOp4;
    ScCursorRefEdit maEdRight4;
    ScRefButton     maRBRight4;
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
    ScRefEdit*      mpEdActive;
    bool            mbDlgLostFocus;

    static const sal_uInt16 EDIT_ROW_COUNT = 4;
    ScCursorRefEdit* mpLeftEdit[EDIT_ROW_COUNT];
    ScRefButton*     mpLeftButton[EDIT_ROW_COUNT];
    ScCursorRefEdit* mpRightEdit[EDIT_ROW_COUNT];
    ScRefButton*     mpRightButton[EDIT_ROW_COUNT];
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
    void    ShowError( bool bCondition, ScRefEdit* pFocus );

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

