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
#ifndef _INSTABLE_HXX
#define _INSTABLE_HXX

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <sfx2/basedlgs.hxx>
#include <actctrl.hxx>
#include <textcontrolcombo.hxx>

class SwWrtShell;
class SwTableAutoFmt;
class SwView;
struct SwInsertTableOptions;


class SwInsTableDlg : public SfxModalDialog
{
    FixedText       aNameFT;
    TableNameEdit   aNameEdit;

    FixedLine       aFL;
    FixedText       aColLbl;
    NumericField    aColEdit;
    FixedText       aRowLbl;
    NumericField    aRowEdit;

    FixedLine       aOptionsFL;
    CheckBox        aHeaderCB;
    CheckBox        aRepeatHeaderCB;
    FixedText       aRepeatHeaderFT;    // "dummy" to build before and after FT
    FixedText       aRepeatHeaderBeforeFT;
    NumericField    aRepeatHeaderNF;
    FixedText       aRepeatHeaderAfterFT;
    TextControlCombo    aRepeatHeaderCombo;

    CheckBox        aDontSplitCB;
    CheckBox        aBorderCB;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    PushButton      aAutoFmtBtn;

    SwWrtShell*     pShell;
    SwTableAutoFmt* pTAutoFmt;
    sal_Int64       nEnteredValRepeatHeaderNF;

    DECL_LINK( ModifyName, Edit * );
    DECL_LINK( ModifyRowCol, NumericField * );
    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( CheckBoxHdl, CheckBox *pCB = 0 );
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );
    DECL_LINK( ModifyRepeatHeaderNF_Hdl, void* p = 0 );

public:
    SwInsTableDlg( SwView& rView );
    ~SwInsTableDlg();

    void GetValues( String& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                    SwInsertTableOptions& rInsTblOpts, String& rTableAutoFmtName,
                    SwTableAutoFmt *& prTAFmt );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
