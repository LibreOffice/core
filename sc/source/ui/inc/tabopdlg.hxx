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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TABOPDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TABOPDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"

enum ScTabOpErr
{
    TABOPERR_NOFORMULA = 1,
    TABOPERR_NOCOLROW,
    TABOPERR_WRONGFORMULA,
    TABOPERR_WRONGROW,
    TABOPERR_NOCOLFORMULA,
    TABOPERR_WRONGCOL,
    TABOPERR_NOROWFORMULA
};

class ScTabOpDlg : public ScAnyRefDlg
{
public:
                    ScTabOpDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                ScDocument*     pDocument,
                                const ScRefAddress& rCursorPos );
                    virtual ~ScTabOpDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc ) SAL_OVERRIDE;
    virtual bool    IsRefInputMode() const SAL_OVERRIDE { return true; }
    virtual void    SetActive() SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;

private:
    FixedText* m_pFtFormulaRange;
    formula::RefEdit* m_pEdFormulaRange;
    formula::RefButton* m_pRBFormulaRange;

    FixedText* m_pFtRowCell;
    formula::RefEdit* m_pEdRowCell;
    formula::RefButton* m_pRBRowCell;

    FixedText* m_pFtColCell;
    formula::RefEdit* m_pEdColCell;
    formula::RefButton* m_pRBColCell;

    OKButton*       m_pBtnOk;
    CancelButton*   m_pBtnCancel;

    ScRefAddress    theFormulaCell;
    ScRefAddress    theFormulaEnd;
    ScRefAddress    theRowCell;
    ScRefAddress    theColCell;

    ScDocument*         pDoc;
    const SCTAB         nCurTab;
    formula::RefEdit*   pEdActive;
    bool                bDlgLostFocus;
    const OUString      errMsgNoFormula;
    const OUString      errMsgNoColRow;
    const OUString      errMsgWrongFormula;
    const OUString      errMsgWrongRowCol;
    const OUString      errMsgNoColFormula;
    const OUString      errMsgNoRowFormula;

    void    Init();
    void    RaiseError( ScTabOpErr eError );

    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, void* );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TABOPDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
