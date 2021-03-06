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

enum ScSolverErr
    {
        SOLVERR_NOFORMULA,
        SOLVERR_INVALID_FORMULA,
        SOLVERR_INVALID_VARIABLE,
        SOLVERR_INVALID_TARGETVALUE
    };

class ScSolverDlg : public ScAnyRefDlgController
{
public:
    ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                 ScDocument* pDocument,
                 const ScAddress& aCursorPos );
    virtual ~ScSolverDlg() override;

    virtual void    SetReference( const ScRange& rRef, ScDocument& rDoc ) override;
    virtual bool    IsRefInputMode() const override;
    virtual void    SetActive() override;
    virtual void    Close() override;

private:
    ScAddress       theFormulaCell;
    ScAddress       theVariableCell;
    OUString        theTargetValStr;

    ScDocument*     pDoc;
    const SCTAB     nCurTab;
    bool            bDlgLostFocus;
    const OUString  errMsgInvalidVar;
    const OUString  errMsgInvalidForm;
    const OUString  errMsgNoFormula;
    const OUString  errMsgInvalidVal;

    formula::RefEdit* m_pEdActive;

    std::unique_ptr<weld::Label> m_xFtFormulaCell;
    std::unique_ptr<formula::RefEdit> m_xEdFormulaCell;
    std::unique_ptr<formula::RefButton> m_xRBFormulaCell;

    std::unique_ptr<weld::Entry> m_xEdTargetVal;

    std::unique_ptr<weld::Label> m_xFtVariableCell;
    std::unique_ptr<formula::RefEdit> m_xEdVariableCell;
    std::unique_ptr<formula::RefButton> m_xRBVariableCell;

    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Button> m_xBtnCancel;

    std::shared_ptr<weld::MessageDialog> m_xMessageBox;

    void    Init();
    bool    CheckTargetValue( const OUString& rStrVal );
    void    RaiseError( ScSolverErr eError );

    DECL_LINK( BtnHdl, weld::Button&, void );
    DECL_LINK( GetEditFocusHdl, formula::RefEdit&, void );
    DECL_LINK( LoseEditFocusHdl, formula::RefEdit&, void );

    DECL_LINK( GetButtonFocusHdl, formula::RefButton&, void );
    DECL_LINK( LoseButtonFocusHdl, formula::RefButton&, void );

    DECL_LINK( GetFocusHdl, weld::Widget&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
