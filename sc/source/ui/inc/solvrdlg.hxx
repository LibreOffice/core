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
#if 1

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
    DECL_LINK( LoseFocusHdl, void* );
#endif  // _SOLVERDLG_CXX
};

#endif // SC_SOLVRDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
