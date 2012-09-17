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
#ifndef _SW_CONVERT_HXX
#define _SW_CONVERT_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <sfx2/basedlgs.hxx>
#include <textcontrolcombo.hxx>

class SwTableAutoFmt;
class SwView;
class SwWrtShell;
class NumericField;
struct SwInsertTableOptions;

class SwConvertTableDlg: public SfxModalDialog
{
    RadioButton*    mpTabBtn;
    RadioButton*    mpSemiBtn;
    RadioButton*    mpParaBtn;
    RadioButton*    mpOtherBtn;
    Edit*           mpOtherEd;
    CheckBox*       mpKeepColumn;

    VclContainer*   mpOptions;

    CheckBox*       mpHeaderCB;
    CheckBox*       mpRepeatHeaderCB;

    VclContainer*   mpRepeatRows;
    NumericField*   mpRepeatHeaderNF;

    CheckBox*       mpDontSplitCB;
    CheckBox*       mpBorderCB;
    PushButton*     mpAutoFmtBtn;

    String          sConvertTextTable;
    SwTableAutoFmt* pTAutoFmt;
    SwWrtShell*     pShell;

    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( BtnHdl, Button* );
    DECL_LINK(CheckBoxHdl, void * = 0);
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );

public:
    SwConvertTableDlg( SwView& rView, bool bToTable );
    ~SwConvertTableDlg();

    void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTblOpts,
                    SwTableAutoFmt const*& prTAFmt );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
