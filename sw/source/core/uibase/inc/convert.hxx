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
#ifndef INCLUDED_SW_SOURCE_UI_INC_CONVERT_HXX
#define INCLUDED_SW_SOURCE_UI_INC_CONVERT_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <sfx2/basedlgs.hxx>

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

    OUString        sConvertTextTable;
    SwTableAutoFmt* pTAutoFmt;
    SwWrtShell*     pShell;

    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( BtnHdl, Button* );
    DECL_LINK(CheckBoxHdl, void * = 0);
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );

public:
    SwConvertTableDlg( SwView& rView, bool bToTable );
    virtual ~SwConvertTableDlg();

    void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTblOpts,
                    SwTableAutoFmt const*& prTAFmt );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
