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

#ifndef SC_SOLVRDLG_HXX
#define SC_SOLVRDLG_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"


#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

//----------------------------------------------------------------------------

enum ScSolverErr
    {
        SOLVERR_NOFORMULA,
        SOLVERR_INVALID_FORMULA,
        SOLVERR_INVALID_VARIABLE,
        SOLVERR_INVALID_TARGETVALUE
    };


//============================================================================

class ScSolverDlg : public ScAnyRefDlg
{
public:
                    ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScDocument* pDocument,
                                 ScAddress aCursorPos );
                    ~ScSolverDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    FixedLine       aFlVariables;
    FixedText       aFtFormulaCell;
    formula::RefEdit        aEdFormulaCell;
    formula::RefButton      aRBFormulaCell;

    FixedText       aFtTargetVal;
    Edit            aEdTargetVal;

    FixedText       aFtVariableCell;
    formula::RefEdit        aEdVariableCell;
    formula::RefButton      aRBVariableCell;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    ScAddress       theFormulaCell;
    ScAddress       theVariableCell;
    String          theTargetValStr;

    ScDocument*     pDoc;
    const SCTAB     nCurTab;
    formula::RefEdit*       pEdActive;
    sal_Bool            bDlgLostFocus;
    const String    errMsgInvalidVar;
    const String    errMsgInvalidForm;
    const String    errMsgNoFormula;
    const String    errMsgInvalidVal;


#ifdef _SOLVRDLG_CXX
    void    Init();
    sal_Bool    CheckTargetValue( String& rStrVal );
    void    RaiseError( ScSolverErr eError );

    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, Control* );
#endif  // _SOLVERDLG_CXX
};

#endif // SC_SOLVRDLG_HXX


