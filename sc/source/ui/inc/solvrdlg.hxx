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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SOLVRDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SOLVRDLG_HXX

#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>

enum ScSolverErr
    {
        SOLVERR_NOFORMULA,
        SOLVERR_INVALID_FORMULA,
        SOLVERR_INVALID_VARIABLE,
        SOLVERR_INVALID_TARGETVALUE
    };

class ScSolverDlg : public ScAnyRefDlg
{
public:
                    ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                 ScDocument* pDocument,
                                 ScAddress aCursorPos );
                    virtual ~ScSolverDlg();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual bool    IsRefInputMode() const SAL_OVERRIDE;
    virtual void    SetActive() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

private:
    VclPtr<FixedText>      m_pFtFormulaCell;
    VclPtr<formula::RefEdit>   m_pEdFormulaCell;
    VclPtr<formula::RefButton> m_pRBFormulaCell;

    VclPtr<Edit>           m_pEdTargetVal;

    VclPtr<FixedText>      m_pFtVariableCell;
    VclPtr<formula::RefEdit>   m_pEdVariableCell;
    VclPtr<formula::RefButton> m_pRBVariableCell;

    VclPtr<OKButton>       m_pBtnOk;
    VclPtr<CancelButton>   m_pBtnCancel;

    ScAddress       theFormulaCell;
    ScAddress       theVariableCell;
    OUString        theTargetValStr;

    ScDocument*     pDoc;
    const SCTAB     nCurTab;
    VclPtr<formula::RefEdit>       pEdActive;
    bool            bDlgLostFocus;
    const OUString  errMsgInvalidVar;
    const OUString  errMsgInvalidForm;
    const OUString  errMsgNoFormula;
    const OUString  errMsgInvalidVal;

    void    Init();
    bool    CheckTargetValue( const OUString& rStrVal );
    void    RaiseError( ScSolverErr eError );

    DECL_LINK_TYPED( BtnHdl, Button*, void );
    DECL_LINK_TYPED( GetFocusHdl, Control&, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_SOLVRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
