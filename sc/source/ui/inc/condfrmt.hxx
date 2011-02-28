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


#ifndef SC_CONDFRMT_HXX_
#define SC_CONDFRMT_HXX_

#include "anyrefdg.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>


class ScDocument;
class ScConditionalFormat;


//============================================================================
//  class ScConditionalFormat
//
// Dialog zum Festlegen von bedingten Formaten

class ScConditionalFormatDlg : public ScAnyRefDlg
{
public:
                    ScConditionalFormatDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                            ScDocument* pCurDoc,
                                            const ScConditionalFormat* pCurrentFormat );
                    ~ScConditionalFormatDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void    AddRefEntry();
    virtual sal_Bool    IsRefInputMode() const;
    virtual void    SetActive();
    virtual sal_Bool    Close();

private:
    CheckBox            aCbxCond1;
    ListBox             aLbCond11;
    ListBox             aLbCond12;
    formula::RefEdit            aEdtCond11;
    formula::RefButton          aRbCond11;
    FixedText           aFtCond1And;
    formula::RefEdit            aEdtCond12;
    formula::RefButton          aRbCond12;
    FixedText           aFtCond1Template;
    ListBox             aLbCond1Template;
    PushButton          aBtnNew1;
    CheckBox            aCbxCond2;
    ListBox             aLbCond21;
    ListBox             aLbCond22;
    formula::RefEdit            aEdtCond21;
    formula::RefButton          aRbCond21;
    FixedText           aFtCond2And;
    formula::RefEdit            aEdtCond22;
    formula::RefButton          aRbCond22;
    FixedText           aFtCond2Template;
    ListBox             aLbCond2Template;
    PushButton          aBtnNew2;
    CheckBox            aCbxCond3;
    ListBox             aLbCond31;
    ListBox             aLbCond32;
    formula::RefEdit            aEdtCond31;
    formula::RefButton          aRbCond31;
    FixedText           aFtCond3And;
    formula::RefEdit            aEdtCond32;
    formula::RefButton          aRbCond32;
    FixedText           aFtCond3Template;
    ListBox             aLbCond3Template;
    PushButton          aBtnNew3;

    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    Point               aCond1Pos1;
    Point               aCond1Pos2;
    Point               aRBtn1Pos1;
    Point               aRBtn1Pos2;
    Size                aCond1Size1;
    Size                aCond1Size2;
    Size                aCond1Size3;

    Point               aCond2Pos1;
    Point               aCond2Pos2;
    Point               aRBtn2Pos1;
    Point               aRBtn2Pos2;
    Size                aCond2Size1;
    Size                aCond2Size2;
    Size                aCond2Size3;

    Point               aCond3Pos1;
    Point               aCond3Pos2;
    Point               aRBtn3Pos1;
    Point               aRBtn3Pos2;
    Size                aCond3Size1;
    Size                aCond3Size2;
    Size                aCond3Size3;
    FixedLine           aFlSep2;
    FixedLine           aFlSep1;
    formula::RefEdit*           pEdActive;
    sal_Bool                bDlgLostFocus;
    ScDocument*         pDoc;

#ifdef _CONDFRMT_CXX
    void    GetConditionalFormat( ScConditionalFormat& rCndFmt );

    DECL_LINK( ClickCond1Hdl, void * );
    DECL_LINK( ChangeCond11Hdl, void * );
    DECL_LINK( ChangeCond12Hdl, void * );

    DECL_LINK( ClickCond2Hdl, void * );
    DECL_LINK( ChangeCond21Hdl, void * );
    DECL_LINK( ChangeCond22Hdl, void * );

    DECL_LINK( ClickCond3Hdl, void * );
    DECL_LINK( ChangeCond31Hdl, void * );
    DECL_LINK( ChangeCond32Hdl, void * );

    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, Control* );
    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( NewBtnHdl, PushButton* );
#endif // _CONDFRMT_CXX
};

#endif // SC_CONDFRMT_HXX_


