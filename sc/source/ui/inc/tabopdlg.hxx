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

#ifndef SC_TABOPDLG_HXX
#define SC_TABOPDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include "global.hxx"
#include "address.hxx"
#include "anyrefdg.hxx"


//------------------------------------------------------------------------

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

//========================================================================

class ScTabOpDlg : public ScAnyRefDlg
{
public:
                    ScTabOpDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                ScDocument*     pDocument,
                                const ScRefAddress& rCursorPos );
                    ~ScTabOpDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool    IsRefInputMode() const { return sal_True; }
    virtual void    SetActive();

    virtual sal_Bool    Close();

private:
    FixedLine       aFlVariables;
    FixedText       aFtFormulaRange;
    formula::RefEdit        aEdFormulaRange;
    formula::RefButton      aRBFormulaRange;

    FixedText       aFtRowCell;
    formula::RefEdit        aEdRowCell;
    formula::RefButton      aRBRowCell;

    FixedText       aFtColCell;
    formula::RefEdit        aEdColCell;
    formula::RefButton      aRBColCell;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    ScRefAddress    theFormulaCell;
    ScRefAddress    theFormulaEnd;
    ScRefAddress    theRowCell;
    ScRefAddress    theColCell;

    ScDocument*     pDoc;
    const SCTAB     nCurTab;
    formula::RefEdit*       pEdActive;
    sal_Bool            bDlgLostFocus;
    const String    errMsgNoFormula;
    const String    errMsgNoColRow;
    const String    errMsgWrongFormula;
    const String    errMsgWrongRowCol;
    const String    errMsgNoColFormula;
    const String    errMsgNoRowFormula;

#ifdef _TABOPDLG_CXX
    void    Init();
    void    RaiseError( ScTabOpErr eError );

    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, Control* );
#endif  // _TABOPDLG_CXX
};

#endif // SC_TABOPDLG_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
